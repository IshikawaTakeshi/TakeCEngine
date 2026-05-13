#pragma once
#include "BehaviorNodeView.h"

//=============================================================
// WaitNodeView class
// WaitNode の View。エディタ上で待機時間を直接編集できる。
//=============================================================
class WaitNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	WaitNodeView(float waitTime = 1.0f);
	~WaitNodeView() override = default;

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

	std::string GetNodeType() const override { return "WAIT"; }
	void SaveParameters(BehaviorNodeData& data) const override;
	void LoadParameters(const BehaviorNodeData& data) override;

	//=========================================================
	// accessors
	//=========================================================

	float GetWaitTime() const { return waitTime_; }
	void SetWaitTime(float t) { waitTime_ = t; }

private:

	float waitTime_ = 1.0f; // 待機時間 [s]
};
