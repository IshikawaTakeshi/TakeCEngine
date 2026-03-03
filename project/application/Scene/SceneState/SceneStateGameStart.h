#pragma once
#include "application/Scene/SceneState/GamePlaySceneState.h"

//============================================================================
// SceneStateGameStart class
// ゲーム開始時の状態
//============================================================================
class SceneStateGameStart : public GamePlaySceneState {
public:
  SceneStateGameStart() = default;
  ~SceneStateGameStart() override = default;

  // 初期化
  void Initialize(GamePlayScene *scene) override;
  // 更新
  void Update(GamePlayScene *scene) override;
};
