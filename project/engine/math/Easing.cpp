#include "Easing.h"

float Easing::Lerp(float startPos, float endPos, float easedT) {
	return (1.0f - easedT) * startPos + easedT * endPos;
}

Vector3 Easing::Lerp(Vector3 startPos, Vector3 endPos, float easedT) {
	return {
		(1.0f - easedT) * startPos.x + easedT * endPos.x,
		(1.0f - easedT) * startPos.y + easedT * endPos.y,
		(1.0f - easedT) * startPos.z + easedT * endPos.z
	};
}

Quaternion Easing::Slerp(Quaternion q0, Quaternion q1, float t) {
	float dot = QuaternionMath::Dot(q0, q1);
	if (dot < 0.0f) {
		q0 = -q0;
		dot = -dot;
	}

	//なす角を求める
	float theta = std::acos(dot);

	//thetaとsinを使って補間係数scale0,scale1を求める
	float sinTheta = std::sinf(theta);
	float scale0 = std::sinf((1.0f - t) * theta) / sinTheta;
	float scale1 = std::sinf(t * theta) / sinTheta;

	//それぞれの保管係数を利用して補間後のクォータニオンを求める
	if (dot >= 0.0005f) {
		return (1.0f - t) * q0 + t * q1;
	}
	return scale0 * q0 + scale1 * q1;
}

float Easing::EaseOut(float x) {
	return sinf((x * std::numbers::pi_v<float>) / 2.0f);
}

float Easing::EaseIn(float x) {
	return 1.0f - cosf((x * std::numbers::pi_v<float>) / 2.0f);
}

float Easing::EaseInOut(float x) {
	return -(cosf(std::numbers::pi_v<float> *x) - 1.0f) / 2.0f;
}
