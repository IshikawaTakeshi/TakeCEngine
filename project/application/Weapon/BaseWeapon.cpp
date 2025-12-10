#include "BaseWeapon.h"

// 武器をスケルトンのジョイントに取り付ける
void BaseWeapon::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
}

const WeaponType& BaseWeapon::GetWeaponType() const {
	// 武器タイプを返す
	return weaponType_;
}

float BaseWeapon::GetAttackInterval() const {
	return weaponState_.attackInterval;
}

const Vector3& BaseWeapon::GetTargetPos() const {
	// ターゲットの座標を返す
	return targetPos_;
}

uint32_t BaseWeapon::GetUnitPosition() const {
	// 武器のユニットポジションを返す
	return unitPosition_;
}

float BaseWeapon::GetBulletSpeed() const {
	// 弾速を返す
	return weaponData_.config.bulletSpeed;
}

float BaseWeapon::GetAttackPower() const {
	// 攻撃力を返す
	return weaponData_.config.power;
}

uint32_t BaseWeapon::GetBulletCount() const {
	// 現在の弾数を返す
	return weaponState_.bulletCount;
}

uint32_t BaseWeapon::GetMaxBulletCount() const {
	// 最大弾数を返す
	return weaponData_.config.maxBulletCount;
}

uint32_t BaseWeapon::GetMagazineCount() const {
	// 一度に撃てる弾容量を返す
	return weaponData_.config.maxMagazineCount;
}

uint32_t BaseWeapon::GetRemainingBulletCount() const {
	// 残弾数の取得
	return weaponState_.remainingBulletCount;
}

float BaseWeapon::GetEffectiveRange() const {
	// 有効射程距離を返す
	return weaponData_.config.effectiveRange;
}

bool BaseWeapon::IsCharging() const {
	// チャージ中かどうかを返す
	return weaponState_.isCharging;
}

float BaseWeapon::GetChargeTime() const {
	// チャージ時間を返す
	return weaponState_.chargeTime;
}

float BaseWeapon::GetRequiredChargeTime() const {
	// 必要チャージ時間を返す
	return weaponData_.config.requiredChargeTime;
}

bool BaseWeapon::GetIsAvailable() const {
	return weaponState_.isAvailable;
}

bool BaseWeapon::GetIsReloading() const {
	return weaponState_.isReloading;
}

void BaseWeapon::SetUnitPosition(uint32_t position) {
	unitPosition_ = position;
}

void BaseWeapon::SetBulletSpeed(float speed) {
	// 弾速を設定
	weaponData_.config.bulletSpeed = speed;
}

void BaseWeapon::SetAttackPower(float power) {
	// 攻撃力を設定
	weaponData_.config.power = power;
}

void BaseWeapon::SetBulletCount(int32_t count) {
	// 弾数を設定
	weaponState_.bulletCount = count;
}

void BaseWeapon::SetRemainingBulletCount(int32_t count) {
	// 残弾数を設定
	weaponState_.remainingBulletCount = count;
}

void BaseWeapon::SetEffectiveRange(float range) {
	// 有効射程距離を設定
	weaponData_.config.effectiveRange = range;
}

bool BaseWeapon::CanChargeAttack() const {
	return weaponData_.config.canChargeAttack;
}

bool BaseWeapon::CanMoveShootable() const {
	return weaponData_.config.canMoveShootable;
}

bool BaseWeapon::StopShootOnly() const {
	return weaponData_.config.isStopShootOnly;
}