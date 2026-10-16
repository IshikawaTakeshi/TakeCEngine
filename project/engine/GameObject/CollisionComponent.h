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

	/// <summary>
	///GameObject の Transform だけを使う通常の初期化
	/// </summary>
	void Initialize(std::unique_ptr<Collider> collider);
	/// <summary>
	/// Transform を使いながら旧 GameCharacter 通知も維持します。
	/// </summary>
	void InitializeFromTransform(
		std::unique_ptr<Collider> collider, 
		GameCharacter* legacyOwner);
	/// <summary>
	/// 移行期間用の Object3d ベースの初期化
	/// </summary>
	void Initialize(
		std::unique_ptr<Collider> collider,
		Object3d* collisionObject,
		GameCharacter* legacyOwner = nullptr);

	/// <summary>
	/// 解放処理
	/// </summary>
	void Reset();

	/// <summary>
	/// 判定・キャスト前に現在の変換を形状へ反映する処理
	/// </summary>
	void SynchronizeTransform();

	/// <summary>
	/// 衝突通知を登録されたコールバックへの送信処理
	/// </summary>
	/// <param name="contact"></param>
	void NotifyCollision(const CollisionContact& contact) const;
	/// <summary>
	/// 衝突イベント通知を登録されたコールバックへの送信処理
	/// </summary>
	/// <param name="event"></param>
	void NotifyCollisionEvent(const CollisionEvent& event) const;

public:

	//=============================================================================
	// accessor
	//=============================================================================

	//----- getter ---------------
	Collider* GetCollider() const { return collider_.get(); }
	ColliderId GetColliderId() const { return colliderId_; }
	GameCharacter* GetLegacyOwner() const { return legacyOwner_; }
	Vector3 GetLocalOffset() const;
	std::uint32_t GetCollisionLayer() const;
	std::uint32_t GetCollisionMask() const { return collisionMask_; }

	//----- setter ---------------
	void SetCollisionCallback(CollisionCallback callback) {
		callback_ = std::move(callback);
	}

	void SetCollisionEventCallback(CollisionEventCallback callback) {
		eventCallback_ = std::move(callback);
	}

	void SetLocalOffset(const Vector3& offset);
	void SetCollisionLayer(std::uint32_t layer);
	bool IsTrigger() const { return isTrigger_; }
	void SetTrigger(bool trigger) { isTrigger_ = trigger; }
	void SetCollisionMask(std::uint32_t mask) { collisionMask_ = mask; }

protected:
	void Update(const ComponentUpdateContext& context) override;
	void OnDetach() override;

private:
	void PrepareCollider(std::unique_ptr<Collider> collider, GameCharacter* legacyOwner);
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
