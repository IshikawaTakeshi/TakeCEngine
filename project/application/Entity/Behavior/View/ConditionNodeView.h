#pragma once
#include "BehaviorNodeView.h"
#include <string>

//=============================================================
// ConditionNodeView class
//=============================================================
class ConditionNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	/// <param name="field"></param>
	/// <param name="op"></param>
	/// <param name="value"></param>
	ConditionNodeView(
		const std::string& field = "",
		const std::string& op = ">=",
		float value = 0.0f);
	~ConditionNodeView() override = default;

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// ノードの内容描画
	/// </summary>
	void draw() override;

	//=========================================================
	// accsesser
	//=========================================================

	const std::string& GetField() const { return field_; }
	void SetField(const std::string& field) { field_ = field; }

	const std::string& GetOp() const { return op_; }
	void SetOp(const std::string& op) { op_ = op; }

	float GetValue() const { return threshold; }
	void SetValue(float value) { threshold = value; }

private:
	std::string field_;  // 比較対象（"energy", "hp", "distance" 等）
	std::string op_;     // 比較演算子（">=", "<=", ">", "<", "==", "!="）
	float threshold;        // 閾値
};
