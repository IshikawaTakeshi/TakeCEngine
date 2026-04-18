#pragma once
#include "application/Entity/State/BaseGameCharacterState.h"
#include <memory>
#include <unordered_map>

class baseInputProvider;
class AnimatorController;
class AnimationMapper;

//============================================================================
// GameCharacterStateManager class
//============================================================================
class GameCharacterStateManager {
public:
	GameCharacterStateManager() = default;
	~GameCharacterStateManager() = default;

	// ステートの初期化
	void Initialize(baseInputProvider* moveDirectionProvider);
	void InitializeStates(PlayableCharacterInfo& characterContext);
	// ステートの更新
	void Update(PlayableCharacterInfo& characterContext);
	void UpdateImGui();
	// ステートの遷移リクエスト
	void RequestState(State nextBehavior);

	/// <summary>
	/// アニメーションコンポーネントの設定
	/// </summary>
	/// <param name="animatorController">AnimatorControllerへのポインタ</param>
	/// <param name="animationMapper">AnimationMapperへのポインタ</param>
	void SetAnimationComponents(AnimatorController* animatorController, AnimationMapper* animationMapper);

	//----- getter ---------------------------

	// 現在のステートタイプ取得
	State GetCurrentStateType() const { return currentStateType_; }
	// 次のステートタイプ取得
	State GetNextStateType() const { return nextState_; }

private:

	// 遷移先のステートを生成する
	void CreateDefaultStates();

private:
	// 現在のステート
	std::unique_ptr<BaseGameCharacterState> currentState_;
	// ステートのマップ
	std::unordered_map<State, std::unique_ptr<BaseGameCharacterState>> states_;

	// 現在のステートタイプ
	State currentStateType_ = State::NONE;
	// 次のステート
	State nextState_ = State::NONE;
	// ステートが変更されたかどうか
	bool isChanged_ = false;
	// 入力プロバイダ
	baseInputProvider* inputProvider_ = nullptr;

	// アニメーションコントローラ（非所有）
	AnimatorController* animatorController_ = nullptr;
	// アニメーションマッパー（非所有）
	AnimationMapper* animationMapper_ = nullptr;
};