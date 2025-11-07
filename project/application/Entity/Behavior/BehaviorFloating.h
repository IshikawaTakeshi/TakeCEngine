#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// BehaviorFloating class
//============================================================================
class BehaviorFloating : public BaseBehavior {
public:

	BehaviorFloating(baseInputProvider* provider);
	~BehaviorFloating() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

	//重力の強さ
	float gravity_ = 9.8f;
};