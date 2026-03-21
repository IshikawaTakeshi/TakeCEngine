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

private:

	/// 子ノードの数
	int childCount_ = 1;
};
