#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"
#include <functional>

//============================================================================
// ConditionNode class
// 条件を評価し、trueならSuccess、falseならFailure
// → コンボの「発動条件」を表現
//============================================================================
class ConditionNode : public BehaviorNode {
public:

	/// <summary>
	/// コンストラクタとデストラクタ
	/// </summary>
	ConditionNode(const std::string& field,
		const std::string& op,
		float value,
		const std::string& name = "Condition");
	~ConditionNode() override = default;

	//====================================================================================
	// functions
	//====================================================================================

	/// <summary>
	/// ノードの実行
	/// </summary>
	/// <param name="characterInfo"></param>
	/// <returns></returns>
	BehaviorStatus Execute(Blackboard& blackboard) override;

private:

	std::string field_;
	float value_;
	std::function<bool(float, float)> compare_;
};