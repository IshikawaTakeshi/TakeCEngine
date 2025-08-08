#pragma once
#include "engine/Entity/GameCharacter.h"
#include "application/Entity/GameCharacterBehavior.h"
#include <optional>
#include <string>

class BaseBehavior {
public:
	BaseBehavior() = default;
	virtual ~BaseBehavior() = default;
	// 初期化
	virtual void Initialize() = 0;
	// 更新
	virtual void Update() = 0;
};