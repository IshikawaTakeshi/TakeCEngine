#pragma once

#include "engine/AI/AICommand.h"

#include <cstddef>
#include <span>
#include <vector>

namespace TakeC {

/// 個体別の未消費要求。移動/停止は最後の要求が優先、攻撃はFIFOです。
/// Update/LateUpdateと同じスレッドで利用します。
class AICommandBuffer final {
public:
    static constexpr std::size_t kDefaultCapacity = 64;
    explicit AICommandBuffer(std::size_t capacity = kDefaultCapacity);
    AICommandBuffer(const AICommandBuffer&) = delete;
    AICommandBuffer& operator=(const AICommandBuffer&) = delete;
    AICommandBuffer(AICommandBuffer&&) = delete;
    AICommandBuffer& operator=(AICommandBuffer&&) = delete;

    /// 満杯ならfalse。入力値が不正ならinvalid_argument。失敗時は既存要求を保持します。
    bool RequestMoveTo(const Vector3& worldPosition, float stoppingDistance = 0.01f);
    bool RequestStopMovement();
    bool RequestAttack(GameObjectId targetId, std::string actionId = "Attack");
    std::vector<AICommand> ConsumeAll();
    void Clear() { commands_.clear(); }
    bool IsEmpty() const { return commands_.empty(); }
    std::size_t GetSize() const { return commands_.size(); }
    std::size_t GetCapacity() const { return capacity_; }
    /// 表示・検査用。参照はBufferの変更または消費まで有効です。
    std::span<const AICommand> GetCommands() const { return commands_; }

private:
    bool RequestMovement(AICommand command);
    std::size_t capacity_;
    std::vector<AICommand> commands_;
};

} // namespace TakeC
