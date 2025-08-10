#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"
#include "application/Entity/GameCharacterInfo.h"

class GameCharacter;
class IMoveDirectionProvider;
class BehaviorRunning : public BaseBehavior {
public:

	BehaviorRunning(IMoveDirectionProvider* provider);
	~BehaviorRunning() override = default;
	void Initialize() override;
	void Update(GameCharcterInfo& characterInfo) override;

private:

	IMoveDirectionProvider* moveDirectionProvider_; // 移動方向を提供するインターフェース
	float deltaTime_ = 0.0f; // デルタタイム
};

