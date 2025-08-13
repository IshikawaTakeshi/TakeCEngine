#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class GameCharacter;
class IMoveDirectionProvider;
class BehaviorRunning : public BaseBehavior {
public:

	BehaviorRunning(IMoveDirectionProvider* provider);
	~BehaviorRunning() override = default;
	void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;
	std::optional<Behavior> TransitionNextBehavior(Behavior nextBehavior) override;

private:

	IMoveDirectionProvider* moveDirectionProvider_; // 移動方向を提供するインターフェース
	float deltaTime_ = 0.0f; // デルタタイム
};

