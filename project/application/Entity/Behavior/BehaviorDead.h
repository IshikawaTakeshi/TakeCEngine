#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// BehaviorDead class
//============================================================================
class BehaviorDead : public BaseBehavior {
public:
	BehaviorDead(baseInputProvider* provider);
	~BehaviorDead() override = default;

	// 初期化
	void Initialize([[maybe_unused]] GameCharacterContext& characterInfo) override;
	// 更新
	void Update(GameCharacterContext& characterInfo) override; 

private:
	// 重力の強さ
	float gravity_ = 9.8f;
};