#include "BehaviorTreeAssetValidator.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace TakeC {

void BehaviorTreeValidationResult::AddError(
	std::string code,
	std::string message,
	std::string nodePath,
	int nodeUID) {
	issues_.push_back({BehaviorTreeIssueSeverity::Error, std::move(code), std::move(message), std::move(nodePath), nodeUID});
	++errorCount_;
}

void BehaviorTreeValidationResult::AddWarning(
	std::string code,
	std::string message,
	std::string nodePath,
	int nodeUID) {
	issues_.push_back({BehaviorTreeIssueSeverity::Warning, std::move(code), std::move(message), std::move(nodePath), nodeUID});
}

BehaviorTreeValidationResult BehaviorTreeAssetValidator::Validate(
	const BehaviorTreeAsset& asset,
	const BehaviorNodeRegistry& registry) {
	BehaviorTreeValidationResult result;

	if (asset.formatVersion <= 0 || asset.formatVersion > BehaviorTreeAsset::kCurrentFormatVersion) {
		result.AddError(
			"UNSUPPORTED_FORMAT_VERSION",
			"BehaviorTreeAsset formatVersion is not supported.");
	}
	if (asset.name.empty()) {
		result.AddWarning("EMPTY_ASSET_NAME", "BehaviorTreeAsset name is empty.");
	}

	std::unordered_map<int, std::string> runtimeUidPaths;
	std::function<void(const BehaviorNodeData&, const std::string&)> validateRuntimeNode;
	validateRuntimeNode = [&](const BehaviorNodeData& node, const std::string& path) {
		if (node.nodeType.empty()) {
			result.AddError("EMPTY_NODE_TYPE", "Node type is empty.", path, node.nodeUID);
		} else if (!registry.Contains(node.nodeType)) {
			result.AddError(
				"UNKNOWN_NODE_TYPE",
				"Node type is not registered: " + node.nodeType,
				path,
				node.nodeUID);
		}

		if (node.nodeUID >= 0) {
			auto [it, inserted] = runtimeUidPaths.emplace(node.nodeUID, path);
			if (!inserted) {
				result.AddError(
					"DUPLICATE_RUNTIME_UID",
					"Runtime node UID is also used by " + it->second + ".",
					path,
					node.nodeUID);
			}
		}

		for (std::size_t index = 0; index < node.children.size(); ++index) {
			validateRuntimeNode(node.children[index], path + ".children[" + std::to_string(index) + "]");
		}
	};
	validateRuntimeNode(asset.root, "root");

	std::unordered_map<int, const BehaviorNodeData*> editorNodes;
	for (std::size_t index = 0; index < asset.editor.nodes.size(); ++index) {
		const auto& node = asset.editor.nodes[index].node;
		const std::string path = "editor.nodes[" + std::to_string(index) + "]";
		if (node.nodeUID < 0) {
			result.AddError("INVALID_EDITOR_UID", "Editor node UID must be non-negative.", path, node.nodeUID);
		} else if (!editorNodes.emplace(node.nodeUID, &node).second) {
			result.AddError("DUPLICATE_EDITOR_UID", "Editor node UID is duplicated.", path, node.nodeUID);
		}
		if (node.nodeType.empty() || !registry.Contains(node.nodeType)) {
			result.AddError(
				"UNKNOWN_EDITOR_NODE_TYPE",
				"Editor node type is not registered: " + node.nodeType,
				path,
				node.nodeUID);
		}
	}

	std::unordered_map<int, std::vector<int>> adjacency;
	std::unordered_map<int, int> incomingCounts;
	for (std::size_t index = 0; index < asset.editor.links.size(); ++index) {
		const auto& link = asset.editor.links[index];
		const std::string path = "editor.links[" + std::to_string(index) + "]";
		if (!editorNodes.contains(link.fromNodeUID)) {
			result.AddError("MISSING_LINK_SOURCE", "Link source UID does not exist.", path, link.fromNodeUID);
		}
		if (!editorNodes.contains(link.toNodeUID)) {
			result.AddError("MISSING_LINK_TARGET", "Link target UID does not exist.", path, link.toNodeUID);
		}
		if (link.fromPinIndex < 0 || link.toPinIndex < 0) {
			result.AddError("INVALID_PIN_INDEX", "Link pin index must be non-negative.", path);
		}
		if (link.fromNodeUID == link.toNodeUID) {
			result.AddError("SELF_LINK", "A node cannot link to itself.", path, link.fromNodeUID);
		}
		if (editorNodes.contains(link.fromNodeUID) && editorNodes.contains(link.toNodeUID)) {
			adjacency[link.fromNodeUID].push_back(link.toNodeUID);
			if (++incomingCounts[link.toNodeUID] > 1) {
				result.AddError("MULTIPLE_PARENTS", "Editor node has more than one parent.", path, link.toNodeUID);
			}
		}
	}

	std::unordered_set<int> visiting;
	std::unordered_set<int> visited;
	std::function<void(int)> detectCycle = [&](int uid) {
		if (visiting.contains(uid)) {
			result.AddError("EDITOR_CYCLE", "Editor links contain a cycle.", "editor.links", uid);
			return;
		}
		if (visited.contains(uid)) return;
		visiting.insert(uid);
		for (int childUid : adjacency[uid]) detectCycle(childUid);
		visiting.erase(uid);
		visited.insert(uid);
	};
	for (const auto& [uid, node] : editorNodes) {
		(void)node;
		detectCycle(uid);
	}

	if (!asset.editor.nodes.empty()) {
		for (const auto& [uid, path] : runtimeUidPaths) {
			if (!editorNodes.contains(uid)) {
				result.AddWarning(
					"RUNTIME_NODE_WITHOUT_EDITOR_NODE",
					"Runtime node has no matching editor node.",
					path,
					uid);
			}
		}
	}

	return result;
}

} // namespace TakeC
