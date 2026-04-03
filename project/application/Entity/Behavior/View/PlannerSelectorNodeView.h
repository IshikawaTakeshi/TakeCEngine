#pragma once
#include "BehaviorNodeView.h"

//=============================================================
// PlannerSelectorNodeView class
//=============================================================
class PlannerSelectorNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	PlannerSelectorNodeView();
	~PlannerSelectorNodeView() override = default;

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// ノードの内容描画
	/// </summary>
	void draw() override;

	//=========================================================
	// serialization [EXT]
	//=========================================================

	std::string GetNodeType() const override { return "PLANNER_SELECTOR"; }
	void SaveParameters(BehaviorNodeData& data) const override { BehaviorNodeView::SaveParameters(data); }
	void LoadParameters(const BehaviorNodeData& data) override { BehaviorNodeView::LoadParameters(data); }

private:

	/// 子ノードの数
	int childCount_ = 1;
};
