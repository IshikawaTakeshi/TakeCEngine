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

	/// <summary>
	/// パラメータの編集
	/// </summary>
	void DrawInspector() override;

	/// <summary>
	/// 比較演算子の設定
	/// </summary>
	/// <param name="op"></param>
	void SetComparison(const std::string& op);

	/// <summary>
	/// 比較対象の値の設定
	/// </summary>
	/// <param name="value"></param>
	void SetValue(float value) { value_ = value; }

	/// <summary>
	/// 比較対象のフィールドの設定
	/// </summary>
	/// <param name="field"></param>
	void SetField(const std::string& field) { field_ = field; }


private:

	std::string field_;
	std::string op_;
	float value_;
	std::function<bool(float, float)> compare_;
};