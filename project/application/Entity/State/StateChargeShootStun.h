#pragma once
#include "application/Entity/State/BaseGameCharacterState.h"

// 前方宣言
class baseInputProvider;

//============================================================================
// StateChargeShootStun class
//============================================================================
class StateChargeShootStun : public BaseGameCharacterState {
public:

	StateChargeShootStun(baseInputProvider* provider);
	~StateChargeShootStun() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

};