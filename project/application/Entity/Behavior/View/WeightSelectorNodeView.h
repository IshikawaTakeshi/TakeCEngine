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

private:

	/// 子ノードの数
	int childCount_ = 1;
};
