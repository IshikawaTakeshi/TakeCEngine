#include "BehaviorNodeRegistry.h"

#include "ActionNode.h"
#include "BehaviorTreeUtil.h"
#include "ConditionNode.h"
#include "PlannerSelectorNode.h"
#include "ScoreConditionNode.h"
#include "SelectorNode.h"
#include "SequenceNode.h"
#include "SetBlackboardBoolNode.h"
#include "SetBlackboardStringNode.h"
#include "WaitBlackboardTimeNode.h"
#include "WaitNode.h"
#include "WeightSelectorNode.h"

#include <algorithm>
#include <utility>

namespace TakeC {

bool BehaviorNodeRegistry::Register(BehaviorNodeRegistration registration, bool replaceExisting) {
	if (registration.typeId.empty() || !registration.creator) {
		return false;
	}

	auto it = registrations_.find(registration.typeId);
	if (it != registrations_.end() && !replaceExisting) {
		return false;
	}

	std::string typeId = registration.typeId;
	registrations_.insert_or_assign(std::move(typeId), std::move(registration));
	return true;
}

std::unique_ptr<BehaviorNode> BehaviorNodeRegistry::Create(const BehaviorNodeData& data) const {
	auto it = registrations_.find(data.nodeType);
	if (it == registrations_.end() || !it->second.creator) {
		return nullptr;
	}

	std::unique_ptr<BehaviorNode> node = it->second.creator(data);
	if (node) {
		node->SetTypeId(it->first);
		node->SetName(data.name);
		node->SetUID(data.nodeUID);
	}
	return node;
}

bool BehaviorNodeRegistry::SerializeParameters(const BehaviorNode& node, BehaviorNodeData& data) const {
	const std::string typeId(node.GetTypeId());
	auto it = registrations_.find(typeId);
	if (it == registrations_.end()) {
		return false;
	}

	data.nodeType = typeId;
	data.name = node.GetName();
	data.nodeUID = node.GetUID();
	if (it->second.serializer) {
		it->second.serializer(node, data);
	}
	return true;
}

bool BehaviorNodeRegistry::Contains(std::string_view typeId) const {
	return registrations_.contains(std::string(typeId));
}

bool BehaviorNodeRegistry::Unregister(std::string_view typeId) {
	return registrations_.erase(std::string(typeId)) != 0;
}

void BehaviorNodeRegistry::Clear() {
	registrations_.clear();
}

std::vector<const BehaviorNodeRegistration*> BehaviorNodeRegistry::Registrations() const {
	std::vector<const BehaviorNodeRegistration*> result;
	result.reserve(registrations_.size());
	for (const auto& [typeId, registration] : registrations_) {
		(void)typeId;
		result.push_back(&registration);
	}
	std::sort(result.begin(), result.end(), [](const auto* lhs, const auto* rhs) {
		if (lhs->category != rhs->category) {
			return lhs->category < rhs->category;
		}
		return lhs->displayName < rhs->displayName;
	});
	return result;
}

void RegisterBuiltInBehaviorNodes(BehaviorNodeRegistry& registry) {
	registry.Register({
		"ACTION", "Action", "Leaf",
		[](const BehaviorNodeData& data) {
			return std::make_unique<ActionNode>(data.targetState, ActionNode::ExecuteCallback{}, ActionNode::ResetCallback{}, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			data.targetState = static_cast<const ActionNode&>(node).GetActionId();
		}
	}, true);

	registry.Register({
		"CONDITION", "Condition", "Leaf",
		[](const BehaviorNodeData& data) {
			return std::make_unique<ConditionNode>(data.field, data.op, data.conditionThreshold, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			const auto& condition = static_cast<const ConditionNode&>(node);
			data.field = condition.GetField();
			data.op = condition.GetOperator();
			data.conditionThreshold = condition.GetThreshold();
		}
	}, true);

	registry.Register({
		"SCORE_CONDITION", "Score Condition", "Leaf",
		[](const BehaviorNodeData& data) {
			return std::make_unique<ScoreConditionNode>(ScoreConditionNode::ScoreFunc{}, data.conditionThreshold, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			data.conditionThreshold = static_cast<const ScoreConditionNode&>(node).GetThreshold();
		}
	}, true);

	registry.Register({
		"SET_BB_BOOL", "Set Blackboard Bool", "Blackboard",
		[](const BehaviorNodeData& data) {
			return std::make_unique<SetBlackboardBoolNode>(data.bbKey, data.bbValue, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			const auto& setNode = static_cast<const SetBlackboardBoolNode&>(node);
			data.bbKey = setNode.GetKey();
			data.bbValue = setNode.GetValue();
		}
	}, true);

	registry.Register({
		"SET_BB_STRING", "Set Blackboard String", "Blackboard",
		[](const BehaviorNodeData& data) {
			return std::make_unique<SetBlackboardStringNode>(data.bbKey, data.bbStringValue, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			const auto& setNode = static_cast<const SetBlackboardStringNode&>(node);
			data.bbKey = setNode.GetKey();
			data.bbStringValue = setNode.GetStringValue();
		}
	}, true);

	registry.Register({
		"WAIT", "Wait", "Leaf",
		[](const BehaviorNodeData& data) {
			return std::make_unique<WaitNode>(data.waitTime, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			data.waitTime = static_cast<const WaitNode&>(node).GetWaitTime();
		}
	}, true);

	registry.Register({
		"WAIT_BB_TIME", "Wait Blackboard Time", "Blackboard",
		[](const BehaviorNodeData& data) {
			return std::make_unique<WaitBlackboardTimeNode>(data.bbKey, data.name);
		},
		[](const BehaviorNode& node, BehaviorNodeData& data) {
			data.bbKey = static_cast<const WaitBlackboardTimeNode&>(node).GetBBKey();
		}
	}, true);

	registry.Register({
		"SEQUENCE", "Sequence", "Composite",
		[](const BehaviorNodeData&) { return std::make_unique<SequenceNode>(); }, {}
	}, true);
	registry.Register({
		"SELECTOR", "Selector", "Composite",
		[](const BehaviorNodeData&) { return std::make_unique<SelectorNode>(); }, {}
	}, true);
	registry.Register({
		"PLANNER_SELECTOR", "Planner Selector", "Composite",
		[](const BehaviorNodeData&) { return std::make_unique<PlannerSelectorNode>(); }, {}
	}, true);
	registry.Register({
		"WEIGHT_SELECTOR", "Weight Selector", "Composite",
		[](const BehaviorNodeData&) { return std::make_unique<WeightSelectorNode>(); }, {}
	}, true);
}

} // namespace TakeC
