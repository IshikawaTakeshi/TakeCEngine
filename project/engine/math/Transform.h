#pragma once
#include "Vector3.h"
#include "Quaternion.h"

//=================================================================================
// Transform struct
//=================================================================================

// オイラー角による変換情報
struct EulerTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// クォータニオンによる変換情報
struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};