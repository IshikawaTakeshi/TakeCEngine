#pragma once
#include "BehaviorNode.h"
#include "application/Entity/Enemy/AIBrainSystem.h"

//============================================================================
// ScoreConditionNode class
// AIBrainSystemのスコアに基づいてコンボを選択
// → ユーティリティAI + ビヘイビアツリーの融合
//============================================================================
class ScoreConditionNode : public BehaviorNode {
public:

	///floatを返す関数の型エイリアス
	using ScoreFunc = std::function<float()>;

	/// <summary>
	/// コンストラクタとデストラクタ
	/// </summary>
	ScoreConditionNode(ScoreFunc scoreFunc, float threshold, const std::string& name = "ScoreCondition");
	~ScoreConditionNode() override = default;

	//====================================================================================
	// functions
	//====================================================================================

	BehaviorStatus Execute(Blackboard& blackboard) override;

private:
	ScoreFunc scoreFunc_;
	float threshold_;
};