#include "SetBlackboardStringNode.h"
#include "application/Entity/Behavior/Blackboard.h"

//====================================================================
// コンストラクタ
//====================================================================
SetBlackboardStringNode::SetBlackboardStringNode(const std::string& key, const std::string& value, const std::string& name)
	: key_(key), value_(value) {
	SetName(name);
}

//====================================================================
// 実行：指定キーに文字列をセットして即 Success を返す
//====================================================================
BehaviorStatus SetBlackboardStringNode::Execute(Blackboard& blackboard) {
	blackboard.SetValue<std::string>(key_, value_);
	return BehaviorStatus::Success;
}

//====================================================================
// リセット：ステートレスなので何もしない
//====================================================================
void SetBlackboardStringNode::Reset() {
}
