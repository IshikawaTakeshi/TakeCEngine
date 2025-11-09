#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class baseInputProvider; // 前方宣言

//============================================================================
// BehaviorStepBoost class
//============================================================================
class BehaviorStepBoost : public BaseBehavior {
public:
	BehaviorStepBoost(baseInputProvider* provider);
	~BehaviorStepBoost() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

	float gravity_ = 9.8f; // 重力の強さ
};