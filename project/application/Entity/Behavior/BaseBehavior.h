#pragma once
#include "engine/Entity/GameCharacter.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include <optional>
#include <string>
#include <map>

using Behavior = GameCharacterBehavior;

class BaseBehavior {
public:
	BaseBehavior() = default;
	virtual ~BaseBehavior() = default;
	// 初期化
	virtual void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) = 0;
	// 更新
	virtual void Update(GameCharacterContext& characterInfo) = 0;
	// 遷移先のビヘイビアチェック
	virtual std::pair<bool,Behavior> TransitionNextBehavior(Behavior nextBehavior);

	bool GetIsTransition() const { return isTransition_; }
	Behavior GetNextBehavior() const { return nextBehavior_; }
	void SetIsTransition(bool isTransition) { isTransition_ = isTransition; }
	void SetNextBehavior(Behavior nextBehavior) { nextBehavior_ = nextBehavior; }

protected:

	bool isTransition_ = false; // 遷移フラグ
	Behavior nextBehavior_ = Behavior::NONE; // 次のビヘイビア
	// デルタタイム
	float deltaTime_ = 0.0f; // デルタタイム
};