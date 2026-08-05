#include "ActionNode.h"

ActionNode::ActionNode(
	std::string actionId,
	ExecuteCallback execute,
	ResetCallback reset,
	std::string name)
	: actionId_(std::move(actionId)),
	  execute_(std::move(execute)),
	  reset_(std::move(reset)) {
	SetName(name);
}

BehaviorStatus ActionNode::Execute(Blackboard& blackboard) {
	if (!execute_) {
		currentStatus_ = BehaviorStatus::Failure;
		return currentStatus_;
	}

	currentStatus_ = execute_(blackboard);
	return currentStatus_;
}

void ActionNode::Reset() {
	BehaviorNode::Reset();
	if (reset_) {
		reset_();
	}
}
