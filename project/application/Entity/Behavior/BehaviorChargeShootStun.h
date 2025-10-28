#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// BehaviorChargeShootStun class
//============================================================================
class BehaviorChargeShootStun : public BaseBehavior {
public:

	BehaviorChargeShootStun(baseInputProvider* provider);
	~BehaviorChargeShootStun() override = default;

	// 初期化
	void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) override;
	// 更新
	void Update(GameCharacterContext& characterInfo) override;

private:

};