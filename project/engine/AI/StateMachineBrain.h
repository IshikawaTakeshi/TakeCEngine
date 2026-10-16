#pragma once

#include "engine/AI/IAgentBrain.h"
#include "engine/AI/IAgentState.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace TakeC {

/// 個体別ステートを所有する汎用FSM。条件判定 → Exit/Enter → 現在StateのTick。
/// 同じゲームループのスレッドで使用し、1Tickに最大1回だけ遷移します。
class StateMachineBrain final : public IAgentBrain {
public:
    using Condition = std::function<bool(const AIContext&)>;
    StateMachineBrain() = default;
    ~StateMachineBrain() override = default;
    StateMachineBrain(const StateMachineBrain&) = delete;
    StateMachineBrain& operator=(const StateMachineBrain&) = delete;
    StateMachineBrain(StateMachineBrain&&) = delete;
    StateMachineBrain& operator=(StateMachineBrain&&) = delete;

    void AddState(std::string id, std::unique_ptr<IAgentState> state);
    /// 登録済みIDを指定。実行中のStateは変えず、次の開始/Reset後に適用します。
    void SetInitialState(std::string id);
    /// 条件は読み取り専用の判断にします。priorityが同じ条件は登録順で優先します。
    void AddTransition(std::string from, std::string to, Condition condition, int priority = 0);
    /// 条件と同じpriorityで比較し、同点では明示要求を優先します。
    /// 同時に複数要求した場合は高priority、同点では最後の要求だけを予約します。
    void RequestTransition(std::string to, int priority = 0);

    /// 未開始/Reset後は空。返したviewは遷移/Reset/Brain破棄まで有効です。
    std::string_view GetCurrentStateId() const { return currentStateId_; }
    std::string_view GetInitialStateId() const { return initialStateId_; }
    bool IsActive() const { return currentState_ != nullptr; }
    std::size_t GetStateCount() const { return states_.size(); }
    std::size_t GetTransitionCount() const { return transitions_.size(); }

    void Start(AIContext& context) override;
    void Tick(AIContext& context) override;
    void Reset(AIContext& context) override;
    // FSMは継続動作します。IsFinishedはIAgentBrainの既定値falseを使います。

private:
    struct Transition {
        std::string from;
        std::string to;
        Condition condition;
        int priority;
    };
    struct TransitionRequest {
        std::string to;
        int priority;
    };
    class ExecutionGuard final {
    public:
        explicit ExecutionGuard(StateMachineBrain& brain);
        ~ExecutionGuard();
        ExecutionGuard(const ExecutionGuard&) = delete;
        ExecutionGuard& operator=(const ExecutionGuard&) = delete;
    private:
        StateMachineBrain& brain_;
    };

    void EnsureNotExecuting() const;
    void EnsureRegistered(std::string_view id) const;
    void EnsureActive(AIContext& context);
    void EnterState(const std::string& id, AIContext& context);
    void ExitCurrentState(AIContext& context);
    void ResetRuntime(AIContext& context);
    bool EvaluateCondition(const Condition& condition, const AIContext& context);

    std::unordered_map<std::string, std::unique_ptr<IAgentState>> states_;
    std::vector<Transition> transitions_;
    std::string initialStateId_;
    std::string currentStateId_;
    IAgentState* currentState_ = nullptr;
    std::optional<TransitionRequest> pendingRequest_;
    bool executing_ = false;
    bool evaluatingCondition_ = false;
};

} // namespace TakeC
