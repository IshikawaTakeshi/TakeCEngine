#pragma once
#include "application/Scene/SceneState/GamePlaySceneState.h"

//============================================================================
// SceneStateGamePlay class
// ゲームプレイ中の状態
//============================================================================
class SceneStateGamePlay : public GamePlaySceneState {
public:
  SceneStateGamePlay() = default;
  ~SceneStateGamePlay() override = default;

  // 初期化
  void Initialize(GamePlayScene *scene) override;
  // 更新
  void Update(GamePlayScene *scene) override;
};
