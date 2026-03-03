#pragma once
#include "application/Scene/SceneState/GamePlayScene/GamePlaySceneState.h"

//============================================================================
// SceneStateGameClear class
// ゲームクリア時の状態
//============================================================================
class SceneStateGameClear : public GamePlaySceneState {
public:
	SceneStateGameClear() = default;
	~SceneStateGameClear() override = default;

	// 初期化
	void Initialize(GamePlayScene* scene) override;
	// 更新
	void Update(GamePlayScene* scene) override;
};
