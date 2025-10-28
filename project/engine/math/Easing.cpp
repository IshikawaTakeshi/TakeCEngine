#include "Easing.h"

//=============================================================================
// 線形補間(float)
//=============================================================================
float Easing::Lerp(float startPos, float endPos, float easedT) {
	return (1.0f - easedT) * startPos + easedT * endPos;
}

//=============================================================================
// 線形補間(Vector3)
//=============================================================================
Vector3 Easing::Lerp(Vector3 startPos, Vector3 endPos, float easedT) {
	return {
		(1.0f - easedT) * startPos.x + easedT * endPos.x,
		(1.0f - easedT) * startPos.y + easedT * endPos.y,
		(1.0f - easedT) * startPos.z + easedT * endPos.z
	};
}

//=============================================================================
// 球面線形補間(Quaternion)
//=============================================================================
Quaternion Easing::Slerp(Quaternion q0, Quaternion q1, float t) {
    float dot = QuaternionMath::Dot(q0, q1);

    // クォータニオンが逆向きの時、最短経路を取るために反転
    if (dot < 0.0f) {
        q0 = -q0;
        dot = -dot;
    }

    // ドット積がほぼ1（角度が小さい）場合、線形補間に切り替える
    const float THRESHOLD = 0.9995f;
    if (dot > THRESHOLD) {
        // LERP (線形補間) を使用
        Quaternion result = (1.0f - t) * q0 + t * q1;
        return QuaternionMath::Normalize(result); // 正規化
    }

    // 通常の SLERP の計算
    float theta = std::acosf(dot);       // なす角
    float sinTheta = std::sinf(theta);   // sin(θ)

    float scale0 = std::sinf((1.0f - t) * theta) / sinTheta;
    float scale1 = std::sinf(t * theta) / sinTheta;

    return scale0 * q0 + scale1 * q1;
}

//=============================================================================
// イージング関数
//=============================================================================
float Easing::EaseOutSine(float x) {
	return sinf((x * std::numbers::pi_v<float>) / 2.0f);
}


float Easing::EaseInSine(float x) {
	return 1.0f - cosf((x * std::numbers::pi_v<float>) / 2.0f);
}

float Easing::EaseInOutSine(float x) {
	return 0.5f * (1.0f - cosf(std::numbers::pi_v<float> * x));
}

float Easing::EaseInCubic(float x) {
	return x * x * x;
}

float Easing::EaseOutCubic(float x) {
	return 1.0f - powf(1.0f - x, 3.0f);
}

float Easing::EaseInExpo(float x) {
	return x == 0.0f ? 0.0f : powf(2.0f, 10.0f * (x - 1.0f));
}

float Easing::EaseOutExpo(float x) {
	return x == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * x);
}

float Easing::UrgentRise(float x) {
	return  std::pow(x, 4.0f);
}

float Easing::GentleRise(float x) {
	return std::sqrt(x);
}

float Easing::EaseInQuad(float x) {
	return x * x;
}

float Easing::EaseOutQuad(float x) {
	return x * (2.0f - x);
}
