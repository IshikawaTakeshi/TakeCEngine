#pragma once
#include "application/Entity/Behavior/SequenceNode.h"
#include "application/Entity/Behavior/SelectorNode.h"
#include "application/Entity/Behavior/PlannerSelectorNode.h"
#include "application/Entity/Behavior/WeightSelectorNode.h"
#include "application/Entity/Behavior/ConditionNode.h"
#include "application/Entity/Behavior/ActionNode.h"
#include "application/Entity/Behavior/BehaviorTreeUtil.h"
#include "application/Entity/State/GameCharacterStateManager.h"
#include "application/Entity/Enemy/AIBrainSystem.h"
#include <memory>

//============================================================================
// ComboFactory: コンボ（行動シーケンス）を組み立てるファクトリ
//============================================================================
class ComboFactory {
public:

	/// <summary>
	/// JSONファイルからコンボセットを読み込み、ビヘイビアツリーを構築
	/// </summary>
	std::unique_ptr<BehaviorNode> LoadComboSetData(
		const std::string& comboSetFilePath,
		GameCharacterStateManager* stateManager);

	/// <summary>
	/// ComboSetData からビヘイビアツリーを構築
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildBehaviorTree(
		const ComboSetData& comboSetData,
		GameCharacterStateManager* stateManager);

	/// <summary>
	/// ノードデータから再帰的にノードを構築
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildNode(
		const BehaviorNodeData& nodeData,
		GameCharacterStateManager* stateManager);

	/// <summary>
	/// ACTION ノードのビルド
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildActionNode(
		const BehaviorNodeData& nodeData,
		GameCharacterStateManager* stateManager);

	/// <summary>
	/// CONDITION ノードのビルド（Blackboard 版）
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildConditionNode(
		const BehaviorNodeData& nodeData);
};

