#pragma once
#include "application/Scene/SceneState/GamePlayScene/GamePlaySceneState.h"

//============================================================================
// SceneStatePause class
// ポーズ時の状態
//============================================================================
/// <summary>
/// ゲームプレイ中のポーズ処理と復帰遷移を担当するシーン状態クラスです。
/// </summary>
class SceneStatePause : public GamePlaySceneState {
public:

	//=======================================================================
	// functions
	//=======================================================================

	// コンストラクタ・デストラクタ
	SceneStatePause() = default;
	~SceneStatePause() override = default;

	// 初期化
	void Initialize(GamePlayScene* scene) override;
	// 更新
	void Update(GamePlayScene* scene) override;


};
