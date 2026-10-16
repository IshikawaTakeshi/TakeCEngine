#pragma once

#include "engine/GameObject/Component.h"
#include "engine/AI/AICommand.h"

namespace TakeC {

class AIControllerComponent;

/// <summary>
/// ControllerのLateUpdateから呼ばれる実行側Component
/// 自身のUpdate/LateUpdateで同じ行動を再実行せず、UpdateExecutionへ実装してください。
/// </summary>
class AICommandHandlerComponent : public Component {
public:
	/// デストラクタ
    ~AICommandHandlerComponent() override = default;
	/// コマンドを受け付けるかどうか
    virtual bool HandleCommand(const AICommand& command) = 0;
	/// 実行処理の更新。CommandSourceを変更する際に呼ばれる
    virtual void UpdateExecution([[maybe_unused]] const ComponentUpdateContext& context) {}
	/// 実行処理のキャンセル。CommandSourceを変更する際に呼ばれる
    virtual void CancelExecution() {}

	/// CommandSourceの設定
    void SetCommandSource(AIControllerComponent* source);
	// CommandSourceの取得
    bool UsesController(const AIControllerComponent& controller) const;

protected:
	// アタッチ時処理
    void OnAttach() final;
	// デタッチ時処理
    void OnDetach() final;
	// Commandのアタッチ時処理
    virtual void OnCommandAttach() {}
	// Commandのデタッチ時処理
    virtual void OnCommandDetach() {}

private:
	// CommandSourceの所有者
    GameObject* commandOwner_ = nullptr;
	// CommandSourceの参照
    AIControllerComponent* source_ = nullptr;
};

} // namespace TakeC
