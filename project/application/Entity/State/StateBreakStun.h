#pragma once
#include "application/Entity/State/BaseGameCharacterState.h"
#include "engine/Utility/Timer.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// StateBreakStun class
//============================================================================
class StateBreakStun : public BaseGameCharacterState {
public:

	//=============================================================================
	// functions
	//=============================================================================

	// コンストラクタ・デストラクタ
	StateBreakStun(baseInputProvider* provider);
	~StateBreakStun() override = default;
	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

	//重力の強さ
	float gravity_ = 9.8f;

	// ブレイクスタンのタイマー
	Timer breakStunTimer_;

};

