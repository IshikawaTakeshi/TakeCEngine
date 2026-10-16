#pragma once
#include "CompositeNode.h"

//====================================================================
// PlannerSelectorNode class
// 目標ごとに評価値を計算し、評価値の分布からランダムにノードを選択して実行する
//====================================================================

/// <summary>
/// ビヘイビアツリーのPlannerSelectorNodeとして判定または処理を実行するクラスです。
/// </summary>
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
	BehaviorStatus Tick(TakeC::AIContext& context) override;
	BehaviorStatus Tick(const TakeC::BehaviorTreeContext& context) override {
		auto mutableContext = context;
		return Tick(mutableContext);
	}

private:
	BehaviorStatus ExecuteImpl(
		Blackboard& blackboard, TakeC::AIContext* context);

private:

	/// <summary>
	/// 子ノードの評価値を計算して、評価値の分布からランダムにノードを選択する
	/// </summary>
	/// <param name="characterInfo"></param>
	/// <returns>選択された子ノードのインデックス</returns>
	size_t SelectNodeIndex(Blackboard& blackboard);

};
