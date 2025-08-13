#pragma once
#include "engine/Entity/GameCharacter.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include <optional>
#include <string>

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
	virtual std::optional<Behavior> TransitionNextBehavior(Behavior nextBehavior);

protected:

	Behavior behaviorRequest_ = Behavior::NONE; // 現在のビヘイビア
};