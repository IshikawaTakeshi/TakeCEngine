#pragma once
#include "application/Scene/GamePlaySceneBehavior.h"
#include <optional>

// 前方宣言
class BaseScene;

//============================================================================
// BaseSceneState class
// シーンの状態基底クラス（汎用）
//============================================================================
class BaseSceneState {
public:

	//=======================================================================
	// functions
	//=======================================================================

	// コンストラクタ・デストラクタ
	BaseSceneState() = default;
	virtual ~BaseSceneState() = default;

	// 初期化
	virtual void Initialize(BaseScene* scene) = 0;
	// 更新
	virtual void Update(BaseScene* scene) = 0;

	// 状態遷移リクエスト
	void RequestTransition(SceneState nextState);

	// 遷移リクエストをリセット
	void ResetTransition();

	//=======================================================================
	// accessors
	//=======================================================================

	//----- getter ---------------------------

	// 遷移リクエストがあるか
	bool HasTransitionRequest() const;
	// 次の状態を取得
	SceneState GetNextState() const;

protected:
	// 遷移リクエスト
	std::optional<SceneState> transitionRequest_ = std::nullopt;
};
