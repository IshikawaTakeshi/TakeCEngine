#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class baseInputProvider; // 前方宣言
class BehaviorJumping : public BaseBehavior {
public:

	BehaviorJumping(baseInputProvider* provider);
	~BehaviorJumping() override = default;
	void Initialize([[maybe_unused]] GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;

private:

	float gravity_ = 9.8f; // 重力の強さ
};