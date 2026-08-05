#include "ComboFactory.h"

#include "CompositeNode.h"
#include "engine/Base/TakeCFrameWork.h"

#include <utility>

namespace TakeC {

ComboFactory::ComboFactory()
	: registry_(&defaultRegistry_) {
	RegisterBuiltInBehaviorNodes(defaultRegistry_);
}

ComboFactory::ComboFactory(BehaviorNodeRegistry& registry)
	: registry_(&registry) {}

std::unique_ptr<BehaviorNode> ComboFactory::LoadBehaviorTreeAsset(const std::string& assetFilePath) {
	const BehaviorTreeAsset asset =
		TakeCFrameWork::GetJsonLoader()->LoadJsonData<BehaviorTreeAsset>(assetFilePath);
	return BuildBehaviorTree(asset);
}

std::unique_ptr<BehaviorNode> ComboFactory::LoadBehaviorTreeAsset(
	const std::filesystem::path& directory,
	const std::string& assetFilePath) {
	return LoadBehaviorTreeAsset(*TakeCFrameWork::GetJsonLoader(), directory, assetFilePath);
}

std::unique_ptr<BehaviorNode> ComboFactory::LoadBehaviorTreeAsset(
	JsonLoader& jsonLoader,
	const std::filesystem::path& directory,
	const std::string& assetFilePath) {
	const BehaviorTreeAsset asset = jsonLoader.LoadJsonDataAt<BehaviorTreeAsset>(directory, assetFilePath);
	return BuildBehaviorTree(asset);
}

std::unique_ptr<BehaviorNode> ComboFactory::BuildBehaviorTree(const BehaviorTreeAsset& asset) const {
	if (!Validate(asset).IsValid()) {
		return nullptr;
	}
	return BuildNode(asset.root);
}

BehaviorTreeValidationResult ComboFactory::Validate(const BehaviorTreeAsset& asset) const {
	return BehaviorTreeAssetValidator::Validate(asset, *registry_);
}

std::unique_ptr<BehaviorNode> ComboFactory::LoadComboSetData(const std::string& comboSetFilePath) {
	const ComboSetData comboSetData =
		TakeCFrameWork::GetJsonLoader()->LoadJsonData<ComboSetData>(comboSetFilePath);
	return BuildBehaviorTree(comboSetData);
}

std::unique_ptr<BehaviorNode> ComboFactory::BuildBehaviorTree(const ComboSetData& comboSetData) const {
	return BuildBehaviorTree(ConvertComboSetToBehaviorTreeAsset(comboSetData));
}

std::unique_ptr<BehaviorNode> ComboFactory::BuildNode(const BehaviorNodeData& nodeData) const {
	std::unique_ptr<BehaviorNode> node = registry_->Create(nodeData);
	if (!node) {
		return nullptr;
	}

	if (auto* composite = dynamic_cast<CompositeNode*>(node.get())) {
		for (const BehaviorNodeData& childData : nodeData.children) {
			std::unique_ptr<BehaviorNode> child = BuildNode(childData);
			if (child) {
				composite->AddChild(std::move(child));
			}
		}
	}

	return node;
}

} // namespace TakeC
