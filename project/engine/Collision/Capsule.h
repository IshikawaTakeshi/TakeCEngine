#pragma once
#include "engine/math/Vector3.h"

/// <summary>
/// 衝突判定で使用するCapsuleの形状または判定結果を表す構造体です。
/// </summary>
struct Capsule {
	Vector3 start; // カプセルの一端の位置
	Vector3 end; // カプセルのもう一端の位置
	float radius;   // カプセルの半径
};