#pragma once

#include "engine/GameObject/Component.h"
#include "engine/AI/AICommand.h"

namespace TakeC {

class AIControllerComponent;

/// ControllerのLateUpdateから呼ばれる実行側Component。
/// 自身のUpdate/LateUpdateで同じ行動を再実行せず、UpdateExecutionへ実装してください。
class AICommandHandlerComponent : public Component {
public:
    ~AICommandHandlerComponent() override = default;
    /// trueで「担当して消費した」を表します。非対応の命令はfalseを返します。
    virtual bool HandleCommand(const AICommand& command) = 0;
    virtual void UpdateExecution([[maybe_unused]] const ComponentUpdateContext& context) {}
    /// Reset/Stop/Brain交換/Detachで呼ばれます。例外を投げず、繰り返し呼べる処理にします。
    virtual void CancelExecution() {}

    /// 未指定なら同じownerの最初のAIControllerを利用。複数AIがある場合に明示します。
    /// GameObjectへ取り付けてから呼んでください。nullptrで既定へ戻します。
    void SetCommandSource(AIControllerComponent* source);
    bool UsesController(const AIControllerComponent& controller) const;

protected:
    void OnAttach() final;
    void OnDetach() final;
    virtual void OnCommandAttach() {}
    virtual void OnCommandDetach() {}

private:
    GameObject* commandOwner_ = nullptr;
    AIControllerComponent* source_ = nullptr;
};

} // namespace TakeC
