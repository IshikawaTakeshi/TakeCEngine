#include "SceneStateManager.h"

//===================================================================================
// 状態を登録
//===================================================================================
void SceneStateManager::RegisterState(SceneState type,
	std::unique_ptr<BaseSceneState> state) {
	states_[type] = std::move(state);
}

//===================================================================================
// 初期状態を設定して初期化
//===================================================================================
void SceneStateManager::Initialize(SceneState initialState, BaseScene* scene) {

	auto it = states_.find(initialState);
	if (it != states_.end()) {
		currentState_ = it->second.get();
		currentStateType_ = initialState;
		currentState_->Initialize(scene);
	}
}

//===================================================================================
// 更新処理
//===================================================================================
void SceneStateManager::Update(BaseScene* scene) {

	if (!currentState_) {
		return;
	}

	// 遷移リクエストがある場合は遷移
	if (currentState_->HasTransitionRequest()) {
		SceneState nextState = currentState_->GetNextState();
		TransitionTo(nextState, scene);
	}

	// 現在のステートを更新
	currentState_->Update(scene);
}

//===================================================================================
// 指定した状態へ遷移
//===================================================================================
void SceneStateManager::TransitionTo(SceneState nextState, BaseScene* scene) {

	// 現在のステートの遷移リクエストをリセット
	currentState_->ResetTransition();

	// 次のステートに遷移
	auto it = states_.find(nextState);
	if (it != states_.end()) {
		currentState_ = it->second.get();
		currentStateType_ = nextState;
		currentState_->Initialize(scene);
	}
}
