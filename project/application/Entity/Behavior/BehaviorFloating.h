#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"
#include "application/Entity/GameCharacterInfo.h"

class BehaviorFloating : public BaseBehavior {
public:

	BehaviorFloating() = default;
	~BehaviorFloating() override = default;
	void Initialize([[maybe_unused]]GameCharacterInfo& characterInfo) override;
	void Update(GameCharacterInfo& characterInfo) override;

private:

	float deltaTime_ = 0.0f; // デルタタイム
	float gravity_ = 9.8f; // 重力の強さ
};

