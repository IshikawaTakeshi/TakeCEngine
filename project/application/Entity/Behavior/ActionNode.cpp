#include "ActionNode.h"
#include "engine/Utility/StringUtility.h"

//==================================================================================
// コンストラクタ
//==================================================================================
ActionNode::ActionNode(
	GameCharacterState targetState,
	GameCharacterStateManager* stateManager,
	const std::string& name) {

	targetState_ = targetState;
	stateManager_ = stateManager;
	name_ = name;
}

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus ActionNode::Execute(Blackboard&) {
	if (!isStarted_) {
		//ステート遷移をリクエスト
		stateManager_->RequestState(targetState_);
		isStarted_ = true;
	}
	//現在のステートがターゲットと一致した場合、まだ実行中
	if (stateManager_->GetCurrentStateType() == targetState_) {
		currentStatus_ = BehaviorStatus::Running;
		return BehaviorStatus::Running;
	}

	//ステートが変わった場合、アクション完了とみなす
	if (isStarted_ && stateManager_->GetCurrentStateType() != targetState_) {
		Reset(); // 次回の実行に備えて状態をリセット
		currentStatus_ = BehaviorStatus::Success;
		return BehaviorStatus::Success;
	}

	currentStatus_ = BehaviorStatus::Running;
	return BehaviorStatus::Running; // まだ実行中
}

//==================================================================================
// ImGuiを用いたデバッグ用情報の描画
//==================================================================================

void ActionNode::DrawInspector() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::PushID(this);
	
	// 名前
	char nameBuf[128];
	strncpy_s(nameBuf, name_.c_str(), sizeof(nameBuf) - 1);
	if (ImGui::InputText("Node Name", nameBuf, sizeof(nameBuf))) {
		name_ = nameBuf;
	}

	// ターゲットステートの選択 (magic_enum + ImGui::Combo)
	auto values = magic_enum::enum_values<GameCharacterState>();
	int currentIdx = -1;
	std::vector<std::string> names; // string_view の lifetime 問題を避けるため一度 string にする
	std::vector<const char*> comboItems;

	for (size_t i = 0; i < values.size(); ++i) {
		names.push_back(std::string(magic_enum::enum_name(values[i])));
		if (values[i] == targetState_) {
			currentIdx = static_cast<int>(i);
		}
	}
	for (const auto& name : names) {
		comboItems.push_back(name.c_str());
	}

	if (ImGui::Combo("Target State", &currentIdx, comboItems.data(), static_cast<int>(comboItems.size()))) {
		targetState_ = values[currentIdx];
	}
	
	ImGui::PopID();
#endif
}

//==================================================================================
// ノードのリセット（再実行時に状態をクリア）
//==================================================================================
void ActionNode::Reset() {
	BehaviorNode::Reset();
	isStarted_ = false;
}
