#pragma once
#include "application/Entity/Behavior/BaseGameCharacterState.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// StateFloating class
//============================================================================
class StateFloating : public BaseGameCharacterState {
public:

	StateFloating(baseInputProvider* provider);
	~StateFloating() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

	//重力の強さ
	float gravity_ = 9.8f;
};