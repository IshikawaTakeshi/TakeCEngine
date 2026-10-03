#include "Collider.h"

#include <stdexcept>

namespace TakeC {

void Collider::Initialize([[maybe_unused]] Object3d* collisionObject) {
	throw std::logic_error("This Collider does not support legacy Object3d initialization.");
}

void Collider::Update([[maybe_unused]] Object3d* collisionObject) {
	throw std::logic_error("This Collider does not support legacy Object3d updates.");
}

void Collider::Initialize([[maybe_unused]] const Matrix4x4& worldMatrix) {
	throw std::logic_error("This Collider does not support TransformComponent initialization.");
}

void Collider::Update([[maybe_unused]] const Matrix4x4& worldMatrix) {
	throw std::logic_error("This Collider does not support TransformComponent updates.");
}

Collider::WorldPose Collider::MakeWorldPose(const Matrix4x4& worldMatrix) const {
	WorldPose pose;
	pose.basis[0] = { worldMatrix.m[0][0], worldMatrix.m[0][1], worldMatrix.m[0][2] };
	pose.basis[1] = { worldMatrix.m[1][0], worldMatrix.m[1][1], worldMatrix.m[1][2] };
	pose.basis[2] = { worldMatrix.m[2][0], worldMatrix.m[2][1], worldMatrix.m[2][2] };
	pose.center = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };
	pose.center += pose.basis[0] * offset_.x +
		pose.basis[1] * offset_.y + pose.basis[2] * offset_.z;
	return pose;
}

// コライダーの色を取得
Vector4 Collider::GetColor() const {
	return color_;
}

// コライダーの半径を取得
float Collider::GetRadius() const {
	return radius_;
}

//種別IDの取得
CollisionLayer Collider::GetCollisionLayerID() {
	return layerID_;
}

//surfaceTypeの取得
SurfaceType Collider::GetSurfaceType() const {
	return surfaceType_;
}

}
