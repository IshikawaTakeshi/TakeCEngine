#pragma once
#include "application/Scene/SceneState/GamePlaySceneState.h"
#include "engine/Utility/Timer.h"

//============================================================================
// SceneStateEnemyDestroyed class
// 敵撃破時の状態
//============================================================================
class SceneStateEnemyDestroyed : public GamePlaySceneState {
public:
  SceneStateEnemyDestroyed() = default;
  ~SceneStateEnemyDestroyed() override = default;

  // 初期化
  void Initialize(GamePlayScene *scene) override;
  // 更新
  void Update(GamePlayScene *scene) override;

private:
  // 状態遷移用タイマー
  Timer changeBehaviorTimer_;
};
