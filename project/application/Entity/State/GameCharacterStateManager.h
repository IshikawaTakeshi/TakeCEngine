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

	// ビヘイビアの初期化
	void Initialize(baseInputProvider* moveDirectionProvider);
	void InitializeBehaviors(PlayableCharacterInfo& characterContext);
	// ビヘイビアの更新
	void Update(PlayableCharacterInfo& characterContext);
	void UpdateImGui();
	// ビヘイビアの遷移リクエスト
	void RequestBehavior(State nextBehavior);

	/// <summary>
	/// アニメーションコンポーネントの設定
	/// </summary>
	/// <param name="animatorController">AnimatorControllerへのポインタ</param>
	/// <param name="animationMapper">AnimationMapperへのポインタ</param>
	void SetAnimationComponents(AnimatorController* animatorController, AnimationMapper* animationMapper);

	//----- getter ---------------------------

	// 現在のビヘイビアタイプ取得
	State GetCurrentBehaviorType() const { return currentBehaviorType_; }
	// 次のビヘイビアタイプ取得
	State GetNextBehaviorType() const { return nextState_; }

private:

	// 遷移先のビヘイビアを生成する
	void CreateDefaultBehaviors();

private:
	// 現在のビヘイビア
	std::unique_ptr<BaseGameCharacterState> currentBehavior_;
	// ビヘイビアのマップ
	std::unordered_map<State, std::unique_ptr<BaseGameCharacterState>> behaviors_;

	// 現在のビヘイビアタイプ
	State currentBehaviorType_ = State::NONE;
	// 次のビヘイビア
	State nextState_ = State::NONE;
	// ビヘイビアが変更されたかどうか
	bool isChanged_ = false;
	// 入力プロバイダ
	baseInputProvider* inputProvider_ = nullptr;

	// アニメーションコントローラ（非所有）
	AnimatorController* animatorController_ = nullptr;
	// アニメーションマッパー（非所有）
	AnimationMapper* animationMapper_ = nullptr;
};