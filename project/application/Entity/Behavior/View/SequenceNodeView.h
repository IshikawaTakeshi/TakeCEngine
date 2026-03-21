#pragma once
#include "BehaviorNodeView.h"

//=============================================================
// SequenceNodeView class
//=============================================================
class SequenceNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SequenceNodeView();
	~SequenceNodeView() override = default;

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
