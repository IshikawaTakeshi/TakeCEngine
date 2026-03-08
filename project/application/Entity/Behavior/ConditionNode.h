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

	using ConditionFunc = std::function<bool(const PlayableCharacterInfo&)>;

	/// <summary>
	/// コンストラクタとデストラクタ
	/// </summary>
	ConditionNode(ConditionFunc condition, const std::string& name = "Condition");
	~ConditionNode() override = default;

	//====================================================================================
	// functions
	//====================================================================================

	/// <summary>
	/// ノードの実行
	/// </summary>
	/// <param name="characterInfo"></param>
	/// <returns></returns>
	BehaviorStatus Execute(PlayableCharacterInfo& characterInfo) override;

private:

	ConditionFunc condition_;
};