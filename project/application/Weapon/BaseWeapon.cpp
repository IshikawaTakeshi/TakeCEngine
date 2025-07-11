#include "BaseWeapon.h"

void BaseWeapon::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
}

float BaseWeapon::GetChargeTime() const {
	return chargeTime_;
}
