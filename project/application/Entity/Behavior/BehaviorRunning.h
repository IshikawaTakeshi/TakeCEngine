#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"

//前方宣言
class baseInputProvider;

//============================================================================
// BehaviorRunning class
//============================================================================
class BehaviorRunning : public BaseBehavior {
public:

	BehaviorRunning(baseInputProvider* provider);
	~BehaviorRunning() override = default;

	// 初期化
	void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) override;
	// 更新
	void Update(PlayableCharacterInfo& characterInfo) override;

private:

};