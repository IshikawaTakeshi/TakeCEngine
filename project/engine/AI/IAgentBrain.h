#pragma once

#include "engine/AI/AIContext.h"

namespace TakeC {
/// BehaviorTree、FSM、自作AIを同じControllerへ載せるための共通契約。
class IAgentBrain {
public:
    virtual ~IAgentBrain() = default;
    virtual void Start([[maybe_unused]] AIContext& context) {}
    virtual void Tick(AIContext& context) = 0;
    /// 終了処理は繰り返し呼び出せる、例外を投げない処理にしてください。
    virtual void Reset([[maybe_unused]] AIContext& context) {}
    /// trueならControllerはTick後に一時停止します。連続実行AIの既定値はfalse。
    virtual bool IsFinished() const { return false; }
};
} // namespace TakeC
