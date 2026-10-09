#pragma once

#include "engine/BehaviorTree/Blackboard.h"
#include "engine/AI/AICommandBuffer.h"

namespace TakeC {
class GameObject;

/// AI方式に依存しない個体別情報。参照をTickの外へ保存しないでください。
/// deltaTimeは前回の判断からの有効更新時間です。
struct AIContext final {
    GameObject& owner;
    Blackboard& blackboard;
    AICommandBuffer& commands;
    float deltaTime = 0.0f;
};
} // namespace TakeC
