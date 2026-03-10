#include "ConditionNode.h"


ConditionNode::ConditionNode(const std::string& field, const std::string& op, float value, const std::string& name) {

	name_ = name;
	field_ = field;
	value_ = value;

	if (op == ">=")      compare_ = [](float a, float b) { return a >= b; };
	else if (op == "<=") compare_ = [](float a, float b) { return a <= b; };
	else if (op == ">")  compare_ = [](float a, float b) { return a > b; };
	else if (op == "<")  compare_ = [](float a, float b) { return a < b; };
	else if (op == "==") compare_ = [](float a, float b) { return a == b; };
	else if (op == "!=") compare_ = [](float a, float b) { return a != b; };

	// デフォルトは ">=" とする
	else                 compare_ = [](float a, float b) { return a >= b; };
}

BehaviorStatus ConditionNode::Execute(Blackboard& blackboard) {
	// variant の中身が何であっても GetAsFloat() で float に統一して比較
	float current = blackboard.GetAsFloat(field_, 0.0f);
	return compare_(current, value_)
		? BehaviorStatus::Success
		: BehaviorStatus::Failure;
}