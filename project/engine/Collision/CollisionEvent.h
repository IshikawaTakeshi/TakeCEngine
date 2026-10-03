#pragma once

#include "engine/GameObject/GameObject.h"
#include "engine/math/Vector3.h"

#include <atomic>
#include <cstdint>
#include <optional>

namespace TakeC {

using ColliderId = std::uint64_t;
inline constexpr ColliderId kInvalidColliderId = 0;

/// <summary>新旧の登録経路で重複しないコライダー ID を発行します。</summary>
inline ColliderId AllocateColliderId() {
	static std::atomic<ColliderId> nextId{ 1 };
	return nextId.fetch_add(1, std::memory_order_relaxed);
}

/// <summary>同じコライダー組の接触開始、継続、終了を表します。</summary>
enum class CollisionPhase { Enter, Stay, Exit };

/// <summary>判定アルゴリズムが算出できた場合だけ付与する接触情報です。</summary>
struct CollisionContactInfo final {
	Vector3 position{};
	Vector3 normal{}; // self から other へ向かう法線
	float penetration = 0.0f;
};

/// <summary>ポインタを保持せずに衝突相手と通知内容を識別するイベントです。</summary>
struct CollisionEvent final {
	GameObjectId selfObjectId = kInvalidGameObjectId;
	GameObjectId otherObjectId = kInvalidGameObjectId;
	ColliderId selfColliderId = kInvalidColliderId;
	ColliderId otherColliderId = kInvalidColliderId;
	CollisionPhase phase = CollisionPhase::Enter;
	bool isTrigger = false;
	std::optional<CollisionContactInfo> contact;
};

} // namespace TakeC
