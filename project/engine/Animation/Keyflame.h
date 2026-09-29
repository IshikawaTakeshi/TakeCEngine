#pragma once
#include "math/Vector3.h"
#include "math/Quaternion.h"

//============================================================================
// Keyframe struct
//============================================================================

// キーフレーム構造体
/// <summary>
/// Keyframeに必要な値をまとめて保持する構造体です。
/// </summary>
template <typename T>
struct Keyframe {

	float time;
	T value;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;