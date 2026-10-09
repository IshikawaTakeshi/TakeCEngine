#pragma once

#include "engine/GameObject/GameObject.h"
#include "engine/Math/Vector3.h"

#include <string>
#include <variant>

namespace TakeC {

/// 移動先はワールド座標。速度・物理制約は実行Component側で決めます。
struct AIMoveToCommand final {
    Vector3 targetPosition;
    float stoppingDistance = 0.01f;
};

struct AIStopMovementCommand final {};

/// 生ポインタではなくIDを渡します。対象の生存・射程・クールダウンは実行側で検証します。
struct AIAttackCommand final {
    GameObjectId targetId = kInvalidGameObjectId;
    std::string actionId = "Attack";
};

using AICommand = std::variant<AIMoveToCommand, AIStopMovementCommand, AIAttackCommand>;

} // namespace TakeC
