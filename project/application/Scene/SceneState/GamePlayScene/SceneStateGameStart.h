#pragma once
#include "application/Scene/SceneState/GamePlayScene/GamePlaySceneState.h"

//============================================================================
// SceneStateGameStart class
// ゲーム開始時の状態
//============================================================================
/// <summary>
/// ゲーム開始時の演出と通常プレイへの遷移を担当するシーン状態クラスです。
/// </summary>
class SceneStateGameStart : public GamePlaySceneState {
public:

	//=======================================================================
	// functions
	//=======================================================================

	// コンストラクタ・デストラクタ
	SceneStateGameStart() = default;
	~SceneStateGameStart() override = default;

	// 初期化
	void Initialize(GamePlayScene* scene) override;
	// 更新
	void Update(GamePlayScene* scene) override;
};
