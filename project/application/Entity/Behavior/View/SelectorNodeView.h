#pragma once
#include "BehaviorNodeView.h"

//=============================================================
// SelectorNodeView class
//=============================================================
class SelectorNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SelectorNodeView();
	~SelectorNodeView() override = default;

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

	std::string GetNodeType() const override { return "SELECTOR"; }
	void SaveParameters(BehaviorNodeData& data) const override { BehaviorNodeView::SaveParameters(data); }
	void LoadParameters(const BehaviorNodeData& data) override { BehaviorNodeView::LoadParameters(data); }

private:

	/// 子ノードの数
	int childCount_ = 1;
};