#include "ConditionNode.h"

ConditionNode::ConditionNode(ConditionFunc condition, const std::string& name) {
	condition_ = std::move(condition);
	name_ = name;
}

BehaviorStatus ConditionNode::Execute(PlayableCharacterInfo& characterInfo) {
	if (condition_(characterInfo)) {
		return BehaviorStatus::Success;
	}
	else {
		return BehaviorStatus::Failure;
	}
}