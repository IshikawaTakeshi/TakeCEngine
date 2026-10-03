#pragma once

#include "engine/GameObject/Component.h"
#include "engine/Collision/CollisionEvent.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

class GameCharacter;

namespace TakeC {

class Collider;
class DirectXCommon;
class Object3d;

/// <summary>
/// 衝突した相手と、その所有者を通知するための情報です。
/// </summary>
struct CollisionContact final {
	Collider* otherCollider = nullptr;
	GameObject* otherObject = nullptr;
	GameCharacter* otherCharacter = nullptr;
};

/// <summary>
/// 既存のColliderをGameObjectに所有させ、衝突登録と更新を管理します。
/// </summary>
class CollisionComponent final : public Component {
public:
	using CollisionCallback = std::function<void(const CollisionContact&)>;
	using CollisionEventCallback = std::function<void(const CollisionEvent&)>;

	CollisionComponent() = default;
	~CollisionComponent() override;

	void Initialize(
		std::unique_ptr<Collider> collider,
		DirectXCommon* dxCommon,
		Object3d* collisionObject,
		GameCharacter* legacyOwner = nullptr);
	void Reset();

	Collider* GetCollider() const { return collider_.get(); }
	ColliderId GetColliderId() const { return colliderId_; }
	GameCharacter* GetLegacyOwner() const { return legacyOwner_; }
	void SetCollisionCallback(CollisionCallback callback) {
		callback_ = std::move(callback);
	}
	void NotifyCollision(const CollisionContact& contact) const;
	void SetCollisionEventCallback(CollisionEventCallback callback) {
		eventCallback_ = std::move(callback);
	}
	void NotifyCollisionEvent(const CollisionEvent& event) const;

	Vector3 GetLocalOffset() const;
	void SetLocalOffset(const Vector3& offset);
	std::uint32_t GetCollisionLayer() const;
	void SetCollisionLayer(std::uint32_t layer);
	bool IsTrigger() const { return isTrigger_; }
	void SetTrigger(bool trigger) { isTrigger_ = trigger; }

	std::uint32_t GetCollisionMask() const { return collisionMask_; }
	void SetCollisionMask(std::uint32_t mask) { collisionMask_ = mask; }

protected:
	void Update(const ComponentUpdateContext& context) override;
	void OnDetach() override;

private:
	std::unique_ptr<Collider> collider_;
	Object3d* collisionObject_ = nullptr;
	GameCharacter* legacyOwner_ = nullptr;
	CollisionCallback callback_;
	CollisionEventCallback eventCallback_;
	ColliderId colliderId_ = kInvalidColliderId;
	Vector3 localOffset_{};
	std::uint32_t collisionLayer_ = 0;
	std::uint32_t collisionMask_ = 0xffffffffu;
	bool isTrigger_ = false;
	bool hasLocalOffsetOverride_ = false;
	bool hasLayerOverride_ = false;
};

} // namespace TakeC
