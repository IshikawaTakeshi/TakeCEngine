#pragma once
#include "BehaviorNodeView.h"

//=============================================================
// WeightSelectorNodeView class
//=============================================================
class WeightSelectorNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	WeightSelectorNodeView();
	~WeightSelectorNodeView() override = default;

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

	std::string GetNodeType() const override { return "WEIGHT_SELECTOR"; }
	void SaveParameters(BehaviorNodeData& data) const override { BehaviorNodeView::SaveParameters(data); }
	void LoadParameters(const BehaviorNodeData& data) override { BehaviorNodeView::LoadParameters(data); }

private:

	/// 子ノードの数
	int childCount_ = 1;
};
