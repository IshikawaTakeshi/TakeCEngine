#pragma once
#include "application/Entity/State/BaseGameCharacterState.h"

class baseInputProvider; // 前方宣言

//============================================================================
// StateStepBoost class
//============================================================================
class StateStepBoost : public BaseGameCharacterState {
public:
	StateStepBoost(baseInputProvider* provider);
	~StateStepBoost() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

	float gravity_ = 9.8f; // 重力の強さ
};