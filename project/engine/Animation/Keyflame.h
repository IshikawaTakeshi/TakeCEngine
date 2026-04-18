#pragma once
#include "Vector3.h"
#include "Quaternion.h"

//============================================================================
// Keyframe struct
//============================================================================

// キーフレーム構造体
template <typename T>
struct Keyframe {

	float time;
	T value;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;