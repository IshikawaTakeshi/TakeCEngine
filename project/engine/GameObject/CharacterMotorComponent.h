#pragma once

#include "engine/GameObject/Component.h"
#include "engine/Math/Vector3.h"

namespace TakeC {

class TransformComponent;

/// <summary>
/// ワールド座標の移動目標へTransformを直線移動させる実行コンポーネントです。
/// AIの判断や経路探索は行いません。
/// </summary>
class CharacterMotorComponent final : public Component {
public:
    void SetSpeed(float unitsPerSecond);
    float GetSpeed() const noexcept { return speed_; }

    /// 目標に到達した場合true。親Transformがある場合もワールド座標で指定します。
    bool MoveToward(const Vector3& worldTarget, float stoppingDistance, float deltaTime);

    /// 既存のAIMovementComponentと同じ移動計算を共有するための入口です。
    static bool StepTransform(
        TransformComponent& transform, const Vector3& worldTarget,
        float speed, float stoppingDistance, float deltaTime);

private:
    float speed_ = 1.0f;
};

} // namespace TakeC
