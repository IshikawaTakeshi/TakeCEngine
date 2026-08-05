#include "ActionNodeView.h"
#include "engine/BehaviorTree/ActionNode.h"

//====================================================================
// コンストラクタ
//====================================================================
ActionNodeView::ActionNodeView(const std::string& actionId) {

	actionId_ = actionId;

	// すでに "Action:" が付いている場合は重ねない
	std::string title = actionId_;
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

	// ノード固有の描画: アクションIDの表示・編集
	if (logicNode_) {
		// ロジックノードがある場合はそのインスペクタ（ActionNode::DrawInspector）が
		// インタラクティブなUIを出すため、ここではステート名を表示するだけにする
		auto* action = static_cast<ActionNode*>(logicNode_);
		actionId_ = action->GetActionId();

		// タイトルの同期
		if (nodeCustomName_ == "UnnamedNode" || nodeCustomName_.empty() || nodeCustomName_ == "Action:NONE" || nodeCustomName_ == "ACTION" || nodeCustomName_.starts_with("Action:")) {
			nodeCustomName_ = "Action:" + actionId_;
			setTitle(nodeCustomName_);
		}

		ImGui::Text("Action ID: %s", actionId_.c_str());
	}
	else {
		// 紐づくロジックノードがない場合はエディタ上で編集可能にする
		ImGui::PushItemWidth(120.0f);

		char buffer[256];
		strncpy_s(buffer, actionId_.c_str(), sizeof(buffer));
		if (ImGui::InputText("Action ID", buffer, sizeof(buffer))) {
			actionId_ = buffer;
			if (nodeCustomName_ == "Action:NONE" || nodeCustomName_ == "UnnamedNode" || nodeCustomName_.empty() || nodeCustomName_.starts_with("Action:")) {
				nodeCustomName_ = "Action:" + actionId_;
				setTitle(nodeCustomName_);
			}
		}

		ImGui::PopItemWidth();
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
		const_cast<ActionNodeView*>(this)->actionId_ = action->GetActionId();
	}
	BehaviorNodeView::SaveParameters(data);
	data.targetState = actionId_;
}

void ActionNodeView::LoadParameters(const BehaviorNodeData& data) {
	BehaviorNodeView::LoadParameters(data);
	actionId_ = data.targetState;
	if (nodeCustomName_ == "UnnamedNode" || nodeCustomName_.empty() || nodeCustomName_ == "Action:NONE" || nodeCustomName_ == "ACTION" || nodeCustomName_.starts_with("Action:")) {
		nodeCustomName_ = "Action:" + actionId_;
	}
	setTitle(nodeCustomName_);
}
