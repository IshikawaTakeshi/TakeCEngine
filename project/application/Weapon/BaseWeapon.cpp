#include "BaseWeapon.h"

void BaseWeapon::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
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
