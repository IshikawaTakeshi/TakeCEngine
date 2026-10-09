#pragma once

#include "engine/GameObject/AICommandHandlerComponent.h"

#include <optional>

namespace TakeC {

/// CommandBufferから受け取ったワールド移動先を保持し、毎LateUpdateで直線移動します。
/// 物理・衝突回避・経路探索は扱いません。必要なら独自Handlerへ置き換えてください。
class AIMovementComponent final : public AICommandHandlerComponent {
public:
    bool HandleCommand(const AICommand& command) override;
    void UpdateExecution(const ComponentUpdateContext& context) override;
    void CancelExecution() override { targetPosition_.reset(); }
    void SetSpeed(float unitsPerSecond);
    float GetSpeed() const { return speed_; }
    bool IsMoving() const { return targetPosition_.has_value(); }
    const std::optional<Vector3>& GetTargetPosition() const { return targetPosition_; }

private:
    std::optional<Vector3> targetPosition_;
    float speed_ = 1.0f;
    float stoppingDistance_ = 0.01f;
};

} // namespace TakeC
