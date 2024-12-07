#include "Quaternion.h"
#include <cmath>

Quaternion QuaternionMath::Multiply(const Quaternion& q1, const Quaternion& q2) {
    Quaternion result;
    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    return result;
}

Quaternion QuaternionMath::IdentityQuaternion() {
    Quaternion result;
    result.x = 0.0f;
    result.y = 0.0f;
    result.z = 0.0f;
    result.w = 1.0f;
    return result;
}

Quaternion QuaternionMath::Conjugate(const Quaternion& q) {
    Quaternion result;
    result.x = -q.x;
    result.y = -q.y;
    result.z = -q.z;
    result.w = q.w;
    return result;
}

float QuaternionMath::Norm(const Quaternion& q) {
    float result = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    return result;
}

Quaternion QuaternionMath::Normalize(const Quaternion& q) {
    Quaternion result;
    float norm = Norm(q);
    result.x = q.x / norm;
    result.y = q.y / norm;
    result.z = q.z / norm;
    result.w = q.w / norm;
    return result;
}

Quaternion QuaternionMath::Inverse(const Quaternion& q) {
    Quaternion result;
    float norm = Norm(q);
    Quaternion conjugate = Conjugate(q);
    result.x = conjugate.x / powf(norm,2);
    result.y = conjugate.y / powf(norm, 2);
    result.z = conjugate.z / powf(norm, 2);
    result.w = conjugate.w / powf(norm, 2);
    return result;

}
