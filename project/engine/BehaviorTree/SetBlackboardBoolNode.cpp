#include "SetBlackboardBoolNode.h"
#include "Blackboard.h"

//====================================================================
// コンストラクタ
//====================================================================
SetBlackboardBoolNode::SetBlackboardBoolNode(const std::string& key, bool value, const std::string& name)
	: key_(key), value_(value) {
	SetName(name);
}

//====================================================================
// 実行
//====================================================================
BehaviorStatus SetBlackboardBoolNode::Execute(Blackboard& blackboard) {
	// 指定されたキーにbool値をセットする
	blackboard.Set(key_, value_);
	return BehaviorStatus::Success; // 実行は即座に成功する
}

//====================================================================
// リセット
//====================================================================
void SetBlackboardBoolNode::Reset() {
	// ここではステートを持たないので何もしない
}
