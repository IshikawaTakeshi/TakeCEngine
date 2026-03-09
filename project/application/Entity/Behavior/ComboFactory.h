#pragma once
#include "application/Entity/Behavior/SequenceNode.h"
#include "application/Entity/Behavior/SelectorNode.h"
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
	/// コンボの組み立て
	/// </summary>
	/// <param name="comboData">コンボデータ</param>
	/// <returns>組み立てられたコンボのルートノード</returns>
	std::unique_ptr<BehaviorNode> LoadComboSetData(const std::string& comboSetFilePath);

	std::unique_ptr<BehaviorNode> BuildBehaviorTree(const BehaviorNodeData& nodeData, const std::string& rootType);

	std::unique_ptr<BehaviorNode> BuildNode(const BehaviorNodeData& nodeData,
		GameCharacterStateManager* stateManager,
		AIBrainSystem* brain);

	/// <summary>
	///　ACTIONノードのビルド
	/// </summary>
	/// <param name="nodeData"></param>
	/// <param name="stateManager"></param>
	/// <returns></returns>
	std::unique_ptr<BehaviorNode> BuildActionNode(
		const BehaviorNodeData& nodeData,
		GameCharacterStateManager* stateManager);

	/// <summary>
	/// CONDITIONノードのビルド
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildConditionNode(
		const BehaviorNodeData& nodeData,AIBrainSystem* brain);
};

