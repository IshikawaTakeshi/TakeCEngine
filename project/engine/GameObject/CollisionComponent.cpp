#include "engine/GameObject/CollisionComponent.h"

#include "engine/Collision/Collider.h"
#include "engine/Collision/CollisionManager.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/TransformComponent.h"

#include <stdexcept>
#include <utility>

namespace TakeC {

CollisionComponent::~CollisionComponent() {
	Reset();
}

void CollisionComponent::Initialize(std::unique_ptr<Collider> collider) {
	InitializeFromTransform(std::move(collider),nullptr);
}

void CollisionComponent::InitializeFromTransform(
	std::unique_ptr<Collider> collider,
	GameCharacter* legacyOwner) {
	PrepareCollider(std::move(collider), legacyOwner);
	try {
		collider_->Initialize(GetOwner().GetTransform().GetWorldMatrix());
	} catch (...) {
		Reset();
		throw;
	}
	colliderId_ = AllocateColliderId();
	CollisionManager::GetInstance().RegisterCollisionComponent(this);
}

void CollisionComponent::Initialize(
	std::unique_ptr<Collider> collider,
	Object3d* collisionObject,
	GameCharacter* legacyOwner) {
	if (!collisionObject) {
		throw std::invalid_argument("Legacy CollisionComponent initialization requires an Object3d.");
	}
	PrepareCollider(std::move(collider), legacyOwner);
	collisionObject_ = collisionObject;
	try {
		collider_->Initialize(collisionObject_);
	} catch (...) {
		Reset();
		throw;
	}
	colliderId_ = AllocateColliderId();
	CollisionManager::GetInstance().RegisterCollisionComponent(this);
}

void CollisionComponent::PrepareCollider(
	std::unique_ptr<Collider> collider, GameCharacter* legacyOwner) {
	if (!collider) {
		throw std::invalid_argument("CollisionComponent requires a collider.");
	}
	Reset();
	collider_ = std::move(collider);
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
		SynchronizeTransform();
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

void CollisionComponent::SynchronizeTransform() {
	if (!collider_) {
		return;
	}
	if (collisionObject_) {
		collider_->Update(collisionObject_);
	} else {
		collider_->Update(GetOwner().GetTransform().GetWorldMatrix());
	}
}

void CollisionComponent::Update([[maybe_unused]] const ComponentUpdateContext& context) {
	SynchronizeTransform();
}

void CollisionComponent::OnDetach() {
	Reset();
}

} // namespace TakeC
