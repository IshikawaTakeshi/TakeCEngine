#include "ActionNodeView.h"
#include "application/Entity/Behavior/ActionNode.h"
#include "engine/Utility/StringUtility.h"

//====================================================================
// コンストラクタ
//====================================================================
ActionNodeView::ActionNodeView(const std::string& stateName) {

	targetStateName_ = stateName;

	// すでに "Action:" が付いている場合は重ねない
	std::string title = targetStateName_;
	if (title.find("Action:") == std::string::npos) {
		title = "Action:" + title;
	}

	//ノードタイトルの設定
	setTitle(title);
	//ノードの色の設定
	setStyle(ImFlow::NodeStyle::blue());

	//入力ピンの設定
	//親ノードから実行フロー
	auto inPin = addIN<BehaviorStatus>("In", BehaviorStatus::Invalid,
		ImFlow::ConnectionFilter::SameType());
	AddInputPin(inPin.get());

	//リーフノードなので出力ピンなし

}

//====================================================================
// ノードの内容描画
//====================================================================

void ActionNodeView::draw() {

	// 共通部分の描画（状態表示）
	BehaviorNodeView::draw();

	// ノード固有の描画: ターゲットステート名の表示・編集
	if (logicNode_) {
		auto* action = static_cast<ActionNode*>(logicNode_);
		targetStateName_ = std::string(magic_enum::enum_name(action->GetTargetState()));
		ImGui::Text("Target State: %s", targetStateName_.c_str());
	}
	else {
		// 紐づくロジックノードがない場合はエディタ上で編集可能にする
		char buffer[256];
		strncpy_s(buffer, targetStateName_.c_str(), sizeof(buffer));
		if (ImGui::InputText("Target State", buffer, sizeof(buffer))) {
			targetStateName_ = buffer;
			setTitle(targetStateName_);
		}
	}
}

//====================================================================
// シリアライズ [EXT]
//====================================================================
void ActionNodeView::SaveParameters(BehaviorNodeData& data) const {
	// 保存前にLogicノードから最新の状態を同期
	if (logicNode_) {
		auto* action = static_cast<ActionNode*>(logicNode_);
		data.name = action->GetName();
		const_cast<ActionNodeView*>(this)->targetStateName_ = std::string(magic_enum::enum_name(action->GetTargetState()));
	}
	BehaviorNodeView::SaveParameters(data);
	data.targetState = targetStateName_;
}

void ActionNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	targetStateName_ = data.targetState;
	setTitle(targetStateName_);
}
