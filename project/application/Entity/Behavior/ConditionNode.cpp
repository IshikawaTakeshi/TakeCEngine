#include "ConditionNode.h"

//====================================================================================
// コンストラクタ
//====================================================================================
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

//====================================================================================
// ノードの実行
//====================================================================================
BehaviorStatus ConditionNode::Execute(Blackboard& blackboard) {
	// variant の中身が何であっても GetAsFloat() で float に統一して比較
	float current = blackboard.GetAsFloat(field_, 0.0f);
	return compare_(current, value_)
		? BehaviorStatus::Success
		: BehaviorStatus::Failure;
}

//====================================================================================
// パラメータの編集
//====================================================================================
void ConditionNode::DrawInspector() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::PushID(this);

	// フィールド名
	char fieldBuf[128];
	strncpy_s(fieldBuf, field_.c_str(), sizeof(fieldBuf) - 1);
	if (ImGui::InputText("Field", fieldBuf, sizeof(fieldBuf))) {
		field_ = fieldBuf;
	}

	// 比較演算子
	const char* ops[] = { ">=", "<=", ">", "<", "==", "!=" };
	int currentOp = 0;
	for (int i = 0; i < 6; ++i) {
		if (op_ == ops[i]) {
			currentOp = i;
			break;
		}
	}
	if (ImGui::Combo("Op", &currentOp, ops, 6)) {
		op_ = ops[currentOp];
		SetComparison(op_);
	}

	// 比較値
	ImGui::DragFloat("Value", &value_, 0.1f);

	ImGui::PopID();
#endif
}

//====================================================================================
// 比較演算子の設定
//====================================================================================
void ConditionNode::SetComparison(const std::string& op) {
	op_ = op;
	if (op == ">=")      compare_ = [](float a, float b) { return a >= b; };
	else if (op == "<=") compare_ = [](float a, float b) { return a <= b; };
	else if (op == ">")  compare_ = [](float a, float b) { return a > b; };
	else if (op == "<")  compare_ = [](float a, float b) { return a < b; };
	else if (op == "==") compare_ = [](float a, float b) { return a == b; };
	else if (op == "!=") compare_ = [](float a, float b) { return a != b; };
	// デフォルトは ">=" とする
	else                 compare_ = [](float a, float b) { return a >= b; };
}
