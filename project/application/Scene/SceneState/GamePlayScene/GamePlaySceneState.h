#pragma once
#include "application/Scene/SceneState/BaseSceneState.h"

// 前方宣言
class GamePlayScene;

//============================================================================
// GamePlaySceneState class
// GamePlayScene用の中間ステートクラス
// BaseScene* → GamePlayScene* のキャスト変換を吸収する
//============================================================================
class GamePlaySceneState : public BaseSceneState {
public:

	//=======================================================================
	// functions
	//=======================================================================

	// コンストラクタ・デストラクタ
	GamePlaySceneState() = default;
	~GamePlaySceneState() override = default;

	// BaseSceneState の実装（キャスト変換）
	void Initialize(BaseScene* scene) override final;
	void Update(BaseScene* scene) override final;

protected:
	// 派生クラスが実装する関数（GamePlayScene*を受け取る）
	virtual void Initialize(GamePlayScene* scene) = 0;
	virtual void Update(GamePlayScene* scene) = 0;

	// 最初のUpdateかどうかを判定するフラグ
	bool isFirstUpdate_ = true; 
};
