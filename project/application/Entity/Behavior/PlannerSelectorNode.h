#pragma once
#include "application/Entity/Behavior/CompositeNode.h"

//====================================================================
// PlannerSelectorNode class
// 目標ごとに評価値を計算し、評価値の分布からランダムにノードを選択して実行する
//====================================================================

class PlannerSelectorNode : public CompositeNode {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	PlannerSelectorNode() { name_ = "PlannerSelectorNode"; }
	~PlannerSelectorNode() override = default;

	//=================================================================
	// functions
	//=================================================================
	/// <summary>
	/// ノードの実行
	/// </summary>
	/// <param name="characterInfo"></param>
	/// <returns></returns>
	BehaviorStatus Execute(Blackboard& blackboard) override;

private:

	/// <summary>
	/// 子ノードの評価値を計算して、評価値の分布からランダムにノードを選択する
	/// </summary>
	/// <param name="characterInfo"></param>
	/// <returns>選択された子ノードのインデックス</returns>
	size_t SelectNodeIndex(Blackboard& blackboard);

};