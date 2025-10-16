#pragma once
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include <cmath>
#include <numbers>

namespace Easing {

	//===============線形補間====================//

	float Lerp(float startPos, float endPos, float easedT);

	Vector3 Lerp(Vector3 startPos, Vector3 endPos, float easedT);

	//球面線形補間
	Quaternion Slerp(Quaternion q0, Quaternion q1, float t);

	//Sine（正弦関数）
	float EaseOutSine(float x);
	float EaseInSine(float x);

	float EaseInOutSine(float x);

	// Quadratic（2次関数）
	float EaseInQuad(float x);
	float EaseOutQuad(float x);
	// Cubic（3次関数）
	float EaseInCubic(float x);
	float EaseOutCubic(float x);

	// Exponential（指数関数）
	float EaseInExpo(float x);
	float EaseOutExpo(float x);

	//急激な変化をさせる
	float UrgentRise(float x);
	//緩やかな変化をさせる
	float GentleRise(float x);

	enum EasingType {
		LINEAR,
		IN_SINE,
		OUT_SINE,
		IN_OUT_SINE,
		IN_QUAD,
		OUT_QUAD,
		IN_CUBIC,
		OUT_CUBIC,
		IN_EXPO,
		OUT_EXPO,
		URGENT_RISE,
		GENTLE_RISE
	};

	using EasingFunction = std::function<float(float)>;
	inline EasingFunction Ease[] = {[](float t){ return t; },
		EaseInSine,
		EaseOutSine,
		EaseInOutSine,
		EaseInQuad,
		EaseOutQuad,
		EaseInCubic,
		EaseOutCubic,
		EaseInExpo,
		EaseOutExpo,
		UrgentRise,
		GentleRise
	};
};