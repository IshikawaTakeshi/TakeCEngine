#pragma once
#include "BehaviorNodeView.h"
#include <string>

//=============================================================
//BehaviorNodeView class
//=============================================================
/// <summary>
/// ActionNodeをエディター上で表示・編集するクラスです。
/// </summary>
class ActionNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	/// <param name="actionId"></param>
	ActionNodeView(const std::string& actionId = "NONE");
	~ActionNodeView() override = default;

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

	std::string GetNodeType() const override { return "ACTION"; }
	void SaveParameters(BehaviorNodeData& data) const override;
	void LoadParameters(const BehaviorNodeData& data) override;

	//=========================================================
	// accsesser
	//=========================================================

	const std::string& GetActionId() const { return actionId_; }
	void SetActionId(const std::string& actionId) { actionId_ = actionId; }

private:
	std::string actionId_;
};
