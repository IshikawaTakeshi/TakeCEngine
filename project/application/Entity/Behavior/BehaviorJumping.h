#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class BehaviorJumping : public BaseBehavior {
public:

	BehaviorJumping() = default;
	~BehaviorJumping() override = default;
	void Initialize([[maybe_unused]] GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;
	std::optional<GameCharacterBehavior> TransitionNextBehavior(GameCharacterBehavior nextBehavior) override;

private:

	float deltaTime_ = 0.0f; // デルタタイム
	float gravity_ = 9.8f; // 重力の強さ
};