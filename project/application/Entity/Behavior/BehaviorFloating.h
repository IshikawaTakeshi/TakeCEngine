#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class BehaviorFloating : public BaseBehavior {
public:

	BehaviorFloating() = default;
	~BehaviorFloating() override = default;
	void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;
	std::optional<Behavior> TransitionNextBehavior(Behavior nextBehavior) override;
private:

	float deltaTime_ = 0.0f; // デルタタイム
	float gravity_ = 9.8f; // 重力の強さ
};

