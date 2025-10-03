#include "BaseWeapon.h"

void BaseWeapon::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
}

const Vector3& BaseWeapon::GetTargetPos() const {
	// ターゲットの座標を返す
	return targetPos_;
}

uint32_t BaseWeapon::GetUnitPosition() const {
	return unitPosition_;
}

float BaseWeapon::GetBulletSpeed() const {
	// 弾速を返す
	return bulletSpeed_;
}

float BaseWeapon::GetAttackPower() const {
	// 攻撃力を返す
	return damage_;
}

uint32_t BaseWeapon::GetBulletCount() const {
	// 現在の弾数を返す
	return bulletCount_;
}

uint32_t BaseWeapon::GetMaxBulletCount() const {
	// 最大弾数を返す
	return maxBulletCount_;
}

uint32_t BaseWeapon::GetMagazineCount() const {
	// 一度に撃てる弾容量を返す
	return magazineCount_;
}

uint32_t BaseWeapon::GetRemainingBulletCount() const {
	// 残弾数の取得
	return remainingBulletCount_;
}

float BaseWeapon::GetEffectiveRange() const {
	// 有効射程距離を返す
	return effectiveRange_;
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

void BaseWeapon::SetUnitPosition(uint32_t position) {
	unitPosition_ = position;
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

void BaseWeapon::SetRemainingBulletCount(int32_t count) {
	// 残弾数を設定
	remainingBulletCount_ = count;
}

void BaseWeapon::SetEffectiveRange(float range) {
	// 有効射程距離を設定
	effectiveRange_ = range;
}