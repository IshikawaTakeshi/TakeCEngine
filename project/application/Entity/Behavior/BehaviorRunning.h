#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

class GameCharacter;
class IMoveDirectionProvider;
class BehaviorRunning : public BaseBehavior {
public:

	BehaviorRunning(IMoveDirectionProvider* provider);
	~BehaviorRunning() override = default;
	void Initialize() override;
	void Update() override;

private:

	IMoveDirectionProvider* moveDirectionProvider_; // 移動方向を提供するインターフェース
};

