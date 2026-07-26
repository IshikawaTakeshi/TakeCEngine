#pragma once
#include "application/Entity/State/BaseGameCharacterState.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// StateDead class
//============================================================================
/// <summary>
/// キャラクターが戦闘不能になった後の処理を制御する状態クラスです。
/// </summary>
class StateDead : public BaseGameCharacterState {
public:
	StateDead(baseInputProvider* provider);
	~StateDead() override = default;

	// 初期化
	void Initialize([[maybe_unused]] PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override; 

private:
	// 重力の強さ
	float gravity_ = 9.8f;
};