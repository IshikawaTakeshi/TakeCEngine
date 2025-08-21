#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class baseInputProvider; // 前方宣言
class BehaviorChargeShootStun : public BaseBehavior {
public:

	BehaviorChargeShootStun(baseInputProvider* provider);
	~BehaviorChargeShootStun() override = default;
	void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;

private:

};

