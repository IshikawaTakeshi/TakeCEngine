#pragma once
#include "application/Scene/SceneState/GamePlaySceneState.h"

//============================================================================
// SceneStateGameOver class
// ゲームオーバー時の状態
//============================================================================
class SceneStateGameOver : public GamePlaySceneState {
public:
  SceneStateGameOver() = default;
  ~SceneStateGameOver() override = default;

  // 初期化
  void Initialize(GamePlayScene *scene) override;
  // 更新
  void Update(GamePlayScene *scene) override;
};
