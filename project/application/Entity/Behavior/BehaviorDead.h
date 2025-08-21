#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"
class BehaviorDead : public BaseBehavior {
public:
	BehaviorDead() = default;
	~BehaviorDead() override = default;
	void Initialize([[maybe_unused]] GameCharacterContext& characterInfo) override;
	void Update(GameCharacterContext& characterInfo) override; 

private:


};

