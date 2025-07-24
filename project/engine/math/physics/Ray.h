#pragma once
#include "engine/math/Vector3.h"

class Collider;

struct Ray {
	Vector3 origin;  // レイの始点
	Vector3 direction; // レイの方向ベクトル
	float distance; // レイの長さ
};

struct RayCastHit {
	bool isHit; // ヒットしたかどうか
	Vector3 position; // ヒット位置
	Vector3 normal; // ヒット面の法線ベクトル
	float distance; // ヒットまでの距離
	Collider* hitCollider; // ヒットしたコライダのポインタ

	RayCastHit() : isHit(false), distance(0.0f), hitCollider(nullptr) {}
};