#pragma once
#include "engine/math/Vector3.h"

// 前方宣言
class Collider;

//=============================================================================
// Ray.h
//=============================================================================

/// レイ構造体
struct Ray {
	Vector3 origin;  // レイの始点
	Vector3 direction; // レイの方向ベクトル
	float distance; // レイの長さ
};

/// レイキャストヒット情報構造体
struct RayCastHit {
	bool isHit; // ヒットしたかどうか
	Vector3 position; // ヒット位置
	Vector3 normal; // ヒット面の法線ベクトル
	float distance; // ヒットまでの距離
	Collider* hitCollider; // ヒットしたコライダのポインタ
};