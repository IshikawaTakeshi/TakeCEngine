#include "engine/GameObject/CollisionComponent.h"

#include "engine/Collision/Collider.h"
#include "engine/Collision/CollisionManager.h"

#include <stdexcept>
#include <utility>

namespace TakeC {

CollisionComponent::~CollisionComponent() {
	Reset();
}

void CollisionComponent::Initialize(
	std::unique_ptr<Collider> collider,
	DirectXCommon* dxCommon,
	Object3d* collisionObject,
	GameCharacter* legacyOwner) {
	if (!collider || !collisionObject) {
		throw std::invalid_argument("CollisionComponent requires a collider and an Object3d.");
	}

	Reset();
	collider_ = std::move(collider);
	collisionObject_ = collisionObject;
	legacyOwner_ = legacyOwner;
	collider_->SetOwner(legacyOwner_);
	if (hasLocalOffsetOverride_) {
		collider_->SetOffset(localOffset_);
	} else {
		localOffset_ = collider_->GetOffset();
	}
	if (hasLayerOverride_) {
		collider_->SetCollisionLayerID(collisionLayer_);
	} else {
		collisionLayer_ = static_cast<std::uint32_t>(collider_->GetCollisionLayerID());
	}
	collider_->Initialize(dxCommon, collisionObject_);
	colliderId_ = AllocateColliderId();
	CollisionManager::GetInstance().RegisterCollisionComponent(this);
}

void CollisionComponent::Reset() {
	CollisionManager::GetInstance().UnregisterCollisionComponent(this);
	collider_.reset();
	colliderId_ = kInvalidColliderId;
	collisionObject_ = nullptr;
	legacyOwner_ = nullptr;
}

void CollisionComponent::NotifyCollision(const CollisionContact& contact) const {
	const CollisionCallback callback = callback_;
	if (callback) {
		callback(contact);
	}
}

void CollisionComponent::NotifyCollisionEvent(const CollisionEvent& event) const {
	const CollisionEventCallback callback = eventCallback_;
	if (callback) {
		callback(event);
	}
}

Vector3 CollisionComponent::GetLocalOffset() const {
	return collider_ ? collider_->GetOffset() : localOffset_;
}

void CollisionComponent::SetLocalOffset(const Vector3& offset) {
	localOffset_ = offset;
	hasLocalOffsetOverride_ = true;
	if (collider_) {
		collider_->SetOffset(offset);
	}
}

std::uint32_t CollisionComponent::GetCollisionLayer() const {
	return collider_ ? static_cast<std::uint32_t>(collider_->GetCollisionLayerID()) : collisionLayer_;
}

void CollisionComponent::SetCollisionLayer(std::uint32_t layer) {
	collisionLayer_ = layer;
	hasLayerOverride_ = true;
	if (collider_) {
		collider_->SetCollisionLayerID(layer);
	}
}

void CollisionComponent::Update([[maybe_unused]] const ComponentUpdateContext& context) {
	if (collider_ && collisionObject_) {
		collider_->Update(collisionObject_);
	}
}

void CollisionComponent::OnDetach() {
	Reset();
}

} // namespace TakeC
