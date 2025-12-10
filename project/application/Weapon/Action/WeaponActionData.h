#pragma once
#include <string>
#include <variant>
#include <json.hpp>
#include "application/Weapon/Rifle/RifleInfo.h"
#include "application/Weapon/ShotGun/ShotGunInfo.h"
#include "application/Weapon/Launcher/VerticalMissileLauncherInfo.h"

// 武器のアクションデータバリアント型
using WeaponDataVariant = std::variant<
	std::monostate,
	RifleInfo,
	ShotGunInfo,
	VerticalMissileLauncherInfo
>;