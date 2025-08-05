#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class BehaviorRunning : public BaseBehavior {
public:

	BehaviorRunning() = default;
	~BehaviorRunning() override = default;
	void Initialize() override;
	void Update() override;
	void UpdateImGui() override;
	void SetGameCharacter(GameCharacter* character) { gameCharacter_ = character; }
	GameCharacter* GetGameCharacter() const { return gameCharacter_; }
};

