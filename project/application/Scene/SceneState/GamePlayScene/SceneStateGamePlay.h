#pragma once
#include "application/Scene/SceneState/GamePlayScene/GamePlaySceneState.h"

//============================================================================
// SceneStateGamePlay class
// ゲームプレイ中の状態
//============================================================================
/// <summary>
/// ゲームプレイ中の進行、勝敗判定、状態遷移を担当するシーン状態クラスです。
/// </summary>
class SceneStateGamePlay : public GamePlaySceneState {
public:

	//=======================================================================
	// functions
	//=======================================================================

	// コンストラクタ・デストラクタ
	SceneStateGamePlay() = default;
	~SceneStateGamePlay() override = default;

	// 初期化
	void Initialize(GamePlayScene* scene) override;
	// 更新
	void Update(GamePlayScene* scene) override;
};
