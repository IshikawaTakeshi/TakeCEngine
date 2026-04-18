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

	/// <summary>
	/// ノードの実行
	/// </summary>
	/// <param name="blackboard"></param>
	/// <returns></returns>
	BehaviorStatus Execute(Blackboard& blackboard) override;

	void Reset() override {}

	/// <summary>
	/// スコア関数の設定
	/// </summary>
	/// <param name="threshold"></param>
	void SetThreshold(float threshold) { threshold_ = threshold; }

	/// <summary>
	/// スコア関数の設定
	/// </summary>
	/// <param name="scoreFunc"></param>
	void SetScoreFunc(ScoreFunc scoreFunc) { scoreFunc_ = std::move(scoreFunc); }

private:
	ScoreFunc scoreFunc_;
	float threshold_;
};