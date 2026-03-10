#include "ComboFactory.h"
#include "engine/base/TakeCFrameWork.h"

std::unique_ptr<BehaviorNode> ComboFactory::LoadComboSetData(const std::string& comboSetFilePath) {
	return std::unique_ptr<BehaviorNode>();
}

std::unique_ptr<BehaviorNode> ComboFactory::BuildActionNode(
	const BehaviorNodeData& nodeData, 
	GameCharacterStateManager* stateManager) {
	
	// nodeDataから遷移先ステートを取得
	GameCharacterState targetState = StringUtility::StringToEnum<GameCharacterState>(nodeData.targetState);
	// ActionNodeを生成
	auto actionNode = std::make_unique<ActionNode>(stateManager, targetState);
	return actionNode;
}

std::unique_ptr<BehaviorNode> ComboFactory::BuildConditionNode(
	const BehaviorNodeData& nodeData, AIBrainSystem* brain) {
	
	const std::string& field = nodeData.field;
	const std::string& op = nodeData.op;
	float threshold = nodeData.conditionThreshold;
	int scoreIndex = nodeData.scoreIndex;

	//条件に応じたラムダ関数を生成
	std::function<bool(const PlayableCharacterInfo&)> conditionFunc;

	if(field == "energy")

	return std::make_unique<ConditionNode>(std::move(conditionFunc),nodeData.name);
}


