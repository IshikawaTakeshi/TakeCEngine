#pragma once
#include "BehaviorNode.h"
#include <functional>
#include <utility>

//============================================================================
// ScoreConditionNode class
// 登録されたスコア取得関数の結果に基づいて条件を判定
//============================================================================
/// <summary>
/// ビヘイビアツリーのScoreConditionNodeとして判定または処理を実行するクラスです。
/// </summary>
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

	void Reset() override { BehaviorNode::Reset(); }

	/// <summary>
	/// スコア関数の設定
	/// </summary>
	/// <param name="threshold"></param>
	void SetThreshold(float threshold) { threshold_ = threshold; }
	float GetThreshold() const noexcept { return threshold_; }

	/// <summary>
	/// スコア関数の設定
	/// </summary>
	/// <param name="scoreFunc"></param>
	void SetScoreFunc(ScoreFunc scoreFunc) { scoreFunc_ = std::move(scoreFunc); }

private:
	ScoreFunc scoreFunc_;
	float threshold_;
};
