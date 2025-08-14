#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class IMoveDirectionProvider; // 前方宣言
class BehaviorStepBoost : public BaseBehavior {
public:
	BehaviorStepBoost(IMoveDirectionProvider* provider);
	~BehaviorStepBoost() override = default;
	void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;
	std::pair<bool, Behavior> TransitionNextBehavior(Behavior nextBehavior) override;

private:

	IMoveDirectionProvider* moveDirectionProvider_; // 移動方向を提供するインターフェース
	float gravity_ = 9.8f; // 重力の強さ
};

