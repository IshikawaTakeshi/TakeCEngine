#pragma once

#include "BehaviorNode.h"

#include <functional>
#include <string>
#include <utility>

/// <summary>
/// 登録されたコールバックを実行する、ゲーム非依存の汎用アクションノードです。
/// </summary>
class ActionNode : public BehaviorNode {
public:
	using ExecuteCallback = std::function<BehaviorStatus(Blackboard&)>;
	using ResetCallback = std::function<void()>;

	ActionNode(
		std::string actionId = {},
		ExecuteCallback execute = {},
		ResetCallback reset = {},
		std::string name = "Action");
	~ActionNode() override = default;

	BehaviorStatus Execute(Blackboard& blackboard) override;
	void Reset() override;

	const std::string& GetActionId() const noexcept { return actionId_; }
	void SetActionId(std::string actionId) { actionId_ = std::move(actionId); }
	void SetExecuteCallback(ExecuteCallback execute) { execute_ = std::move(execute); }
	void SetResetCallback(ResetCallback reset) { reset_ = std::move(reset); }

private:
	std::string actionId_;
	ExecuteCallback execute_;
	ResetCallback reset_;
};
