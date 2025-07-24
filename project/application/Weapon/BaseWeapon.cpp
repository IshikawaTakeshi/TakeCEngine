#include "BaseWeapon.h"

void BaseWeapon::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
}

const Vector3& BaseWeapon::GetTragetPos() const {
	return targetPos_;
}

float BaseWeapon::GetBulletSpeed() const {
	return bulletSpeed_;
}

float BaseWeapon::GetAttackPower() const {
	return damage_;
}

int32_t BaseWeapon::GetBulletCount() const {
	return bulletCount_;
}

bool BaseWeapon::IsCharging() const {
	// チャージ中かどうかを返す
	return isCharging_;
}

float BaseWeapon::GetChargeTime() const {
	// チャージ時間を返す
	return chargeTime_;
}

float BaseWeapon::GetRequiredChargeTime() const {
	// 必要チャージ時間を返す
	return requiredChargeTime_;
}

void BaseWeapon::SetBulletSpeed(float speed) {
	// 弾速を設定
	bulletSpeed_ = speed;
}

void BaseWeapon::SetAttackPower(float power) {
	// 攻撃力を設定
	damage_ = power;
}

void BaseWeapon::SetBulletCount(int32_t count) {
	// 弾数を設定
	bulletCount_ = count;
}
