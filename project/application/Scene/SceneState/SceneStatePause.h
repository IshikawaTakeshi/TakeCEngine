#pragma once
#include "application/Scene/SceneState/GamePlaySceneState.h"

//============================================================================
// SceneStatePause class
// ポーズ時の状態
//============================================================================
class SceneStatePause : public GamePlaySceneState {
public:
  SceneStatePause() = default;
  ~SceneStatePause() override = default;

  // 初期化
  void Initialize(GamePlayScene *scene) override;
  // 更新
  void Update(GamePlayScene *scene) override;
};
