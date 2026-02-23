#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"
#include <memory>
#include <unordered_map>

class baseInputProvider;
class AnimatorController;
class AnimationMapper;

//============================================================================
// BehaviorManager class
//============================================================================
class BehaviorManager {
public:
	BehaviorManager() = default;
	~BehaviorManager() = default;

	// ビヘイビアの初期化
	void Initialize(baseInputProvider* moveDirectionProvider);
	void InitializeBehaviors(PlayableCharacterInfo& characterContext);
	// ビヘイビアの更新
	void Update(PlayableCharacterInfo& characterContext);
	void UpdateImGui();
	// ビヘイビアの遷移リクエスト
	void RequestBehavior(Behavior nextBehavior);

	/// <summary>
	/// アニメーションコンポーネントの設定
	/// </summary>
	/// <param name="animatorController">AnimatorControllerへのポインタ</param>
	/// <param name="animationMapper">AnimationMapperへのポインタ</param>
	void SetAnimationComponents(AnimatorController* animatorController, AnimationMapper* animationMapper);

	//----- getter ---------------------------

	// 現在のビヘイビアタイプ取得
	Behavior GetCurrentBehaviorType() const { return currentBehaviorType_; }
	// 次のビヘイビアタイプ取得
	Behavior GetNextBehaviorType() const { return nextBehavior_; }

private:

	// 遷移先のビヘイビアを生成する
	void CreateDefaultBehaviors();

private:
	// 現在のビヘイビア
	std::unique_ptr<BaseBehavior> currentBehavior_;
	// ビヘイビアのマップ
	std::unordered_map<Behavior, std::unique_ptr<BaseBehavior>> behaviors_;

	// 現在のビヘイビアタイプ
	Behavior currentBehaviorType_ = Behavior::NONE;
	// 次のビヘイビア
	Behavior nextBehavior_ = Behavior::NONE;
	// ビヘイビアが変更されたかどうか
	bool isChanged_ = false;
	// 入力プロバイダ
	baseInputProvider* inputProvider_ = nullptr;

	// アニメーションコントローラ（非所有）
	AnimatorController* animatorController_ = nullptr;
	// アニメーションマッパー（非所有）
	AnimationMapper* animationMapper_ = nullptr;
};