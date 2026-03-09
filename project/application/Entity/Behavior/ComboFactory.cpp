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
	
	ConditionType conditionType = StringUtility::StringToEnum<ConditionType>(nodeData.conditionType);
	float threshold = nodeData.conditionThreshold;
	int scoreIndex = nodeData.scoreIndex;

	//条件に応じたラムダ関数を生成
	std::function<bool(const PlayableCharacterInfo&)> conditionFunc;

	switch (conditionType) {
	case ConditionType::ENERGY_ABOVE:
		conditionFunc = [threshold](const PlayableCharacterInfo& info) {
			return info.energyInfo.energy >= threshold;
		};
		break;
	case ConditionType::HP_BELOW:
		conditionFunc = [threshold](const PlayableCharacterInfo& info) {
			return info.health <= threshold;
		};
		break;
	case ConditionType::ON_GROUND:
		conditionFunc = [](const PlayableCharacterInfo& info) {
			return info.onGround;
		};
		break;
	case ConditionType::IN_AIR:
		conditionFunc = [](const PlayableCharacterInfo& info) {
			return !info.onGround;
		};
		break;
	case ConditionType::DISTANCE_BELOW:
		conditionFunc = [threshold,brain](const PlayableCharacterInfo& info) {
			return brain->GetDistanceToTarget() <= threshold;
		};
		break;
	case ConditionType::DISTANCE_ABOVE:
		conditionFunc = [threshold,brain](const PlayableCharacterInfo& info) {
			return brain->GetDistanceToTarget() >= threshold;
		};
		break;
	case ConditionType::SCORE_ABOVE:
		conditionFunc = [threshold, brain, scoreIndex](const PlayableCharacterInfo& info) {
			return brain->GetAttackScore(scoreIndex) >= threshold;
		};
	case ConditionType::NOT_OVERHEATED:
		conditionFunc = [](const PlayableCharacterInfo& info) {
			return !info.overHeatInfo.isOverheated;
			};
	default:
		conditionFunc = [](const PlayableCharacterInfo& info) {
			return true; // デフォルトは常にtrue
		};
		break;
	}

	return std::make_unique<ConditionNode>(std::move(conditionFunc),nodeData.name);
}


