#pragma once
#include "math/Vector3.h"

/**
 * @brief 警告の種類
 */
enum class WarningType {
	NORMAL,  // 通常
	MISSILE, // ミサイル
	HIGHPOWER_ATTACK  // バズーカ
};

/**
 * @brief 警告データ
 */
/// <summary>
/// WarningDataに関するデータを保持する構造体です。
/// </summary>
struct WarningData {
	Vector3 position;
	WarningType type;
};
