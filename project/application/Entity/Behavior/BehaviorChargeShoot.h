#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class baseInputProvider;
class BehaviorChargeShoot : public BaseBehavior {
public:

	BehaviorChargeShoot(baseInputProvider* provider);
	~BehaviorChargeShoot() override = default;
	void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override;

private:


};

