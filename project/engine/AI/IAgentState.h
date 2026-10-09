#pragma once

#include "engine/AI/AIContext.h"

namespace TakeC {

/// 個体別ステートの共通契約。判断と命令発行だけを担当し、実行はHandlerへ委ねます。
class IAgentState {
public:
    virtual ~IAgentState() = default;
    virtual void Enter([[maybe_unused]] AIContext& context) {}
    virtual void Tick(AIContext& context) = 0;
    /// Reset・交換・Detachでも呼ばれます。例外を投げない終了処理にしてください。
    virtual void Exit([[maybe_unused]] AIContext& context) {}
};

} // namespace TakeC
