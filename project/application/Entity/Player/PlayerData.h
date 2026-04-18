#pragma once
#include <string>
#include <array>
#include "application/Weapon/WeaponType.h"
#include "application/Entity/WeaponUnit.h"

struct PlayerData {
	std::string useCharacterName; // 使用中のキャラクター名
	std::array<WeaponType, WeaponUnit::Size> equippedWeaponTypes; // 装備中の武器タイプ配列
};