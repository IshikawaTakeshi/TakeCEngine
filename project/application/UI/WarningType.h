#pragma once
#include "math/Vector3.h"

/**
 * @brief 警告の種類
 */
enum class WarningType {
	NORMAL,  // 通常
	MISSILE, // ミサイル
	BAZOOKA  // バズーカ
};

/**
 * @brief 警告データ
 */
struct WarningData {
	Vector3 position;
	WarningType type;
};
