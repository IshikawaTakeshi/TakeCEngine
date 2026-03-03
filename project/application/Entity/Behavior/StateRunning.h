#pragma once
#include "application/Entity/Behavior/BaseGameCharacterState.h"

//前方宣言
class baseInputProvider;

//============================================================================
// StateRunning class
//============================================================================
class StateRunning : public BaseGameCharacterState {
public:

	StateRunning(baseInputProvider* provider);
	~StateRunning() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

};