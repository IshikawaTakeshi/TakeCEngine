#pragma once
#include "Vector3.h"
struct OBB {
	Vector3 center; // 中心座標
	Vector3 axis[3]; // 3つの軸ベクトル
	Vector3 halfSize; // 半径
};