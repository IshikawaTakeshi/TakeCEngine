#include "ComboFactory.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"

//========================================================================
// JSONファイルからコンボセットを読み込み、ツリーを構築
//========================================================================
std::unique_ptr<BehaviorNode> ComboFactory::LoadComboSetData(
	const std::string& comboSetFilePath,
	GameCharacterStateManager* stateManager) {
	ComboSetData comboSetData = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ComboSetData>(comboSetFilePath);
	return BuildBehaviorTree(comboSetData, stateManager);
}

//========================================================================
// ComboSetData からビヘイビアツリーを構築
//========================================================================
std::unique_ptr<BehaviorNode> ComboFactory::BuildBehaviorTree(
	const ComboSetData& comboSetData,
	GameCharacterStateManager* stateManager) {
	// ルートノードの生成
	std::unique_ptr<CompositeNode> root;
	if (comboSetData.rootType == "SEQUENCE") {
		root = std::make_unique<SequenceNode>();
	} else {
		root = std::make_unique<SelectorNode>();
	}
	root->SetName(comboSetData.setName);

	// 各コンボを子として追加
	for (const auto& comboData : comboSetData.combos) {
		auto comboNode = BuildNode(comboData.rootNode, stateManager);
		if (comboNode) {
			comboNode->SetName(comboData.comboName);
			root->AddChild(std::move(comboNode));
		}
	}

	// フォールバック
	root->AddChild(std::make_unique<ActionNode>(
		GameCharacterState::RUNNING, stateManager, "DefaultRunning"));

	return root;
}

//========================================================================
// ノードデータから再帰的にノードを構築
//========================================================================
std::unique_ptr<BehaviorNode> ComboFactory::BuildNode(
	const BehaviorNodeData& nodeData,
	GameCharacterStateManager* stateManager) {
	BehaviorNodeType type = StringUtility::StringToEnum<BehaviorNodeType>(nodeData.nodeType);

	switch (type) {
	case BehaviorNodeType::ACTION:
		return BuildActionNode(nodeData, stateManager);

	case BehaviorNodeType::CONDITION:
		return BuildConditionNode(nodeData);

	case BehaviorNodeType::SEQUENCE:
	{
		auto composite = std::make_unique<SequenceNode>();
		composite->SetName(nodeData.name);
		for (const auto& child : nodeData.children) {
			auto childNode = BuildNode(child, stateManager);
			if (childNode) {
				composite->AddChild(std::move(childNode));
			}
		}
		return composite;
	}

	case BehaviorNodeType::SELECTOR:
	{
		auto composite = std::make_unique<SelectorNode>();
		composite->SetName(nodeData.name);
		for (const auto& child : nodeData.children) {
			auto childNode = BuildNode(child, stateManager);
			if (childNode) {
				composite->AddChild(std::move(childNode));
			}
		}
		return composite;
	}

	default:
		return nullptr;
	}
}

//========================================================================
// ACTIONノードのビルド
//========================================================================
std::unique_ptr<BehaviorNode> ComboFactory::BuildActionNode(
	const BehaviorNodeData& nodeData,
	GameCharacterStateManager* stateManager) {
	GameCharacterState state = StringUtility::StringToEnum<GameCharacterState>(nodeData.targetState);
	return std::make_unique<ActionNode>(state, stateManager, nodeData.name);
}

//========================================================================
// CONDITIONノードのビルド
//========================================================================
std::unique_ptr<BehaviorNode> ComboFactory::BuildConditionNode(
	const BehaviorNodeData& nodeData) {
	// JSONから読んだ field, op, value をそのまま渡すだけ
	return std::make_unique<ConditionNode>(
		nodeData.field,
		nodeData.op,
		nodeData.conditionThreshold,
		nodeData.name
	);
}