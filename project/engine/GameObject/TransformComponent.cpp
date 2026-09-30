#include "engine/GameObject/TransformComponent.h"

#include "engine/Math/MatrixMath.h"

#include <algorithm>
#include <stdexcept>

namespace TakeC {

TransformComponent::TransformComponent()
	: worldMatrix_(MatrixMath::MakeIdentity4x4()) {}

void TransformComponent::SetLocalPosition(const Vector3& position) {
	localPosition_ = position;
	MarkDirty();
}

void TransformComponent::SetLocalRotation(const Quaternion& rotation) {
	constexpr float kMinimumNorm = 0.000001f;
	if (QuaternionMath::Norm(rotation) <= kMinimumNorm) {
		localRotation_ = QuaternionMath::IdentityQuaternion();
	} else {
		localRotation_ = QuaternionMath::Normalize(rotation);
	}
	MarkDirty();
}

void TransformComponent::SetLocalScale(const Vector3& scale) {
	localScale_ = scale;
	MarkDirty();
}

void TransformComponent::SetParent(TransformComponent* parent) {
	if (parent_ == parent) {
		return;
	}
	if (parent == this || WouldCreateCycle(parent)) {
		throw std::invalid_argument("TransformComponent parent would create a cycle.");
	}

	if (parent_ != nullptr) {
		std::erase(parent_->children_, this);
	}
	parent_ = parent;
	if (parent_ != nullptr) {
		parent_->children_.push_back(this);
	}
	MarkDirty();
}

const Matrix4x4& TransformComponent::GetWorldMatrix() const {
	UpdateWorldMatrix();
	return worldMatrix_;
}

Vector3 TransformComponent::GetWorldPosition() const {
	const Matrix4x4& world = GetWorldMatrix();
	return { world.m[3][0], world.m[3][1], world.m[3][2] };
}

void TransformComponent::OnDetach() {
	if (parent_ != nullptr) {
		std::erase(parent_->children_, this);
		parent_ = nullptr;
	}

	const std::vector<TransformComponent*> children = children_;
	children_.clear();
	for (TransformComponent* child : children) {
		if (child != nullptr) {
			child->parent_ = nullptr;
			child->MarkDirty();
		}
	}
}

void TransformComponent::MarkDirty() {
	if (worldMatrixDirty_) {
		return;
	}

	worldMatrixDirty_ = true;
	for (TransformComponent* child : children_) {
		if (child != nullptr) {
			child->MarkDirty();
		}
	}
}

void TransformComponent::UpdateWorldMatrix() const {
	if (!worldMatrixDirty_) {
		return;
	}

	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		localScale_, localRotation_, localPosition_);
	if (parent_ != nullptr) {
		worldMatrix_ = MatrixMath::Multiply(worldMatrix_, parent_->GetWorldMatrix());
	}
	worldMatrixDirty_ = false;
}

bool TransformComponent::WouldCreateCycle(const TransformComponent* parent) const {
	for (const TransformComponent* ancestor = parent;
		ancestor != nullptr;
		ancestor = ancestor->parent_) {
		if (ancestor == this) {
			return true;
		}
	}
	return false;
}

} // namespace TakeC
