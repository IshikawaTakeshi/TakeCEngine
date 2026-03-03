#include "BaseSceneState.h"

//===================================================================================
// 状態遷移リクエスト
//===================================================================================
void BaseSceneState::RequestTransition(SceneState nextState) {
  transitionRequest_ = nextState;
}

//===================================================================================
// 遷移リクエストがあるか
//===================================================================================
bool BaseSceneState::HasTransitionRequest() const {
  return transitionRequest_.has_value();
}

//===================================================================================
// 次の状態を取得
//===================================================================================
SceneState BaseSceneState::GetNextState() const {
  if (transitionRequest_.has_value()) {
    return transitionRequest_.value();
  }
  return SceneState::GAMESTART; // デフォルト値
}

//===================================================================================
// 遷移リクエストをリセット
//===================================================================================
void BaseSceneState::ResetTransition() { transitionRequest_ = std::nullopt; }
