#include "BehaviorTreeUtil.h"

namespace {

void RemoveEditorLayout(nlohmann::json& nodeJson) {
	nodeJson.erase("posX");
	nodeJson.erase("posY");
	nodeJson.erase("sizeW");
	nodeJson.erase("sizeH");
	if (nodeJson.contains("children") && nodeJson["children"].is_array()) {
		for (auto& child : nodeJson["children"]) {
			RemoveEditorLayout(child);
		}
	}
}

} // namespace

void to_json(nlohmann::json& j, const BehaviorEditorNodeData& data) {
	nlohmann::json nodeJson;
	to_json(nodeJson, data.node);
	RemoveEditorLayout(nodeJson);
	j = nlohmann::json{
		{"node", std::move(nodeJson)},
		{"posX", data.posX},
		{"posY", data.posY},
		{"sizeW", data.sizeW},
		{"sizeH", data.sizeH}
	};
}

void from_json(const nlohmann::json& j, BehaviorEditorNodeData& data) {
	data.node = j.value("node", BehaviorNodeData{});
	data.posX = j.value("posX", 0.0f);
	data.posY = j.value("posY", 0.0f);
	data.sizeW = j.value("sizeW", 0.0f);
	data.sizeH = j.value("sizeH", 0.0f);
}

//============================================================================
// BehaviorTreeAsset の JSON 変換関数
//============================================================================

void to_json(nlohmann::json& j, const BehaviorTreeAsset& data) {
	nlohmann::json rootJson;
	to_json(rootJson, data.root);
	RemoveEditorLayout(rootJson);

	j = nlohmann::json{
		{"formatVersion", data.formatVersion},
		{"name", data.name},
		{"root", std::move(rootJson)},
		{"editor", data.editor}
	};
}

void from_json(const nlohmann::json& j, BehaviorTreeAsset& data) {
	// 旧ComboSet JSONを新APIから直接読み込んだ場合も移行できるようにする。
	if (j.contains("combos") || j.contains("setName")) {
		data = ConvertComboSetToBehaviorTreeAsset(j.get<ComboSetData>());
		return;
	}

	data.formatVersion = j.value("formatVersion", BehaviorTreeAsset::kCurrentFormatVersion);
	data.name = j.value("name", "BehaviorTree");
	data.root = j.value("root", BehaviorNodeData{});
	data.editor = j.value("editor", BehaviorTreeEditorData{});
}

BehaviorTreeAsset ConvertComboSetToBehaviorTreeAsset(const ComboSetData& legacyData) {
	BehaviorTreeAsset asset;
	asset.name = legacyData.setName;
	asset.root.name = legacyData.setName;
	asset.root.nodeType = legacyData.rootType.empty() ? "SELECTOR" : legacyData.rootType;

	for (const auto& combo : legacyData.combos) {
		auto root = combo.rootNode;
		if (!combo.comboName.empty()) {
			root.name = combo.comboName;
		}
		asset.root.children.push_back(std::move(root));
	}

	asset.editor.nodes.reserve(legacyData.editorNodes.size());
	for (const auto& legacyNode : legacyData.editorNodes) {
		BehaviorEditorNodeData editorNode;
		editorNode.node = legacyNode;
		editorNode.posX = legacyNode.posX;
		editorNode.posY = legacyNode.posY;
		editorNode.sizeW = legacyNode.sizeW;
		editorNode.sizeH = legacyNode.sizeH;
		asset.editor.nodes.push_back(std::move(editorNode));
	}
	asset.editor.links = legacyData.editorLinks;
	return asset;
}

ComboSetData ConvertBehaviorTreeAssetToComboSet(const BehaviorTreeAsset& asset) {
	ComboSetData legacyData;
	legacyData.setName = asset.name;
	legacyData.rootType = asset.root.nodeType;
	for (const auto& child : asset.root.children) {
		legacyData.combos.push_back(ComboData{child.name, child});
	}

	legacyData.editorNodes.reserve(asset.editor.nodes.size());
	for (const auto& editorNode : asset.editor.nodes) {
		auto legacyNode = editorNode.node;
		legacyNode.posX = editorNode.posX;
		legacyNode.posY = editorNode.posY;
		legacyNode.sizeW = editorNode.sizeW;
		legacyNode.sizeH = editorNode.sizeH;
		legacyData.editorNodes.push_back(std::move(legacyNode));
	}
	legacyData.editorLinks = asset.editor.links;
	return legacyData;
}


//============================================================================
// ComboSetData の JSON 変換関数
//============================================================================

void from_json(const nlohmann::json& j, ComboSetData& data) {

	// 必須項目 (存在しない場合は例外)
	j.at("setName").get_to(data.setName);
	j.at("combos").get_to(data.combos);

	// 任意項目（なければデフォルト値を維持）
	if (j.contains("rootType") && !j["rootType"].is_null()) {
		j.at("rootType").get_to(data.rootType);
	}

	if (j.contains("editorNodes") && !j["editorNodes"].is_null()) {
		j.at("editorNodes").get_to(data.editorNodes);
	} else {
		data.editorNodes.clear();
	}

	if (j.contains("editorLinks") && !j["editorLinks"].is_null()) {
		j.at("editorLinks").get_to(data.editorLinks);
	} else {
		data.editorLinks.clear();
	}
}

void to_json(nlohmann::json& j, const ComboSetData& data) {
	j = nlohmann::json::object();
	j["setName"] = data.setName;

	j["combos"] = nlohmann::json::array();
	for (const auto& combo : data.combos) {
		nlohmann::json combo_json;
		to_json(combo_json, combo); // 明示的に変換関数を呼ぶ
		j["combos"].push_back(combo_json);
	}

	j["rootType"] = data.rootType;

	j["editorNodes"] = nlohmann::json::array();
	for (const auto& node : data.editorNodes) {
		nlohmann::json node_json;
		to_json(node_json, node); // 明示的に変換関数を呼ぶ
		j["editorNodes"].push_back(node_json);
	}

	j["editorLinks"] = nlohmann::json::array();
	for (const auto& link : data.editorLinks) {
		nlohmann::json link_json;
		to_json(link_json, link); // 明示的に変換関数を呼ぶ
		j["editorLinks"].push_back(link_json);
	}
}

//============================================================================
// BehaviorNodeData の JSON 変換関数
//============================================================================

void from_json(const nlohmann::json& j, BehaviorNodeData& data) {

	data.name = j.value("name", "UnnamedNode");
	data.nodeType = j.value("nodeType", "ACTION");
	data.targetState = j.value("targetState", "");

	data.field = j.value("field", "");
	data.op = j.value("op", ">=");
	data.conditionThreshold = j.value("conditionThreshold", 0.0f);

	data.children.clear();
	if (j.contains("children") && j["children"].is_array()) {
		for (const auto& child_json : j["children"]) {
			BehaviorNodeData childData;
			from_json(child_json, childData); // 明示的に変換関数を呼ぶ
			data.children.push_back(childData);
		}
	}

	data.posX = j.value("posX", 0.0f);
	data.posY = j.value("posY", 0.0f);

	data.nodeUID = j.value("nodeUID", -1);
	data.sizeW = j.value("sizeW", 0.0f);
	data.sizeH = j.value("sizeH", 0.0f);
	data.bbKey = j.value("bbKey", "");
	data.bbValue = j.value("bbValue", false);
	data.bbStringValue = j.value("bbStringValue", "");
	data.waitTime = j.value("waitTime", 1.0f);
	data.properties = j.value("properties", nlohmann::json::object());
}

void to_json(nlohmann::json& j, const BehaviorNodeData& data) {

	j["name"] = data.name;
	j["nodeType"] = data.nodeType;
	j["targetState"] = data.targetState;

	j["field"] = data.field;
	j["op"] = data.op;
	j["conditionThreshold"] = data.conditionThreshold;

	j["children"] = nlohmann::json::array();
	for (const auto& child : data.children) {
		nlohmann::json child_json;
		to_json(child_json, child); // 明示的に変換関数を呼ぶ
		j["children"].push_back(child_json);
	}
	j["posX"] = data.posX;
	j["posY"] = data.posY;

	j["nodeUID"] = data.nodeUID;
	j["sizeW"] = data.sizeW;
	j["sizeH"] = data.sizeH;
	j["bbKey"] = data.bbKey;
	j["bbValue"] = data.bbValue;
	j["bbStringValue"] = data.bbStringValue;
	j["waitTime"] = data.waitTime;
	j["properties"] = data.properties;
}
