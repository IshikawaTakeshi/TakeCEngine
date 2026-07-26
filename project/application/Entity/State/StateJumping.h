#pragma once
#include "application/Entity/State/BaseGameCharacterState.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// StateJumping class
//============================================================================
/// <summary>
/// キャラクターのジャンプ中の移動と着地遷移を制御する状態クラスです。
/// </summary>
class StateJumping : public BaseGameCharacterState {
public:

	StateJumping(baseInputProvider* provider);
	~StateJumping() override = default;

	// 初期化
	void Initialize([[maybe_unused]] PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

	// 重力の強さ
	float gravity_ = 9.8f;
};