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

private:

	/// 子ノードの数
	int childCount_ = 1;
};