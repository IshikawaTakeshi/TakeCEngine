#pragma once
#include "application/Scene/SceneState/GamePlayScene/GamePlaySceneState.h"

//============================================================================
// SceneStateGameOver class
// ゲームオーバー時の状態
//============================================================================
/// <summary>
/// 敗北時の演出とゲームオーバー遷移を担当するシーン状態クラスです。
/// </summary>
class SceneStateGameOver : public GamePlaySceneState {
public:

	//=======================================================================
	// functions
	//=======================================================================

	// コンストラクタ・デストラクタ
	SceneStateGameOver() = default;
	~SceneStateGameOver() override = default;

	// 初期化
	void Initialize(GamePlayScene* scene) override;
	// 更新
	void Update(GamePlayScene* scene) override;
};
