#pragma once

#include "engine/Math/Vector3.h"

#include <optional>
#include <vector>

namespace TakeC {

/// <summary>
/// ワールド座標の始点と終点から同期的に経路を返すサービスの共通インターフェースです。
/// 経路は終点を含み、探索不能ならnulloptを返します。
/// </summary>
class IPathfindingService {
public:
    virtual ~IPathfindingService() = default;
    virtual std::optional<std::vector<Vector3>> FindPath(
        const Vector3& start, const Vector3& goal) = 0;
};

} // namespace TakeC
