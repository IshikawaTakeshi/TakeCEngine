#pragma once
#include "BehaviorNodeView.h"
#include <string>

//=============================================================
// SetBlackboardBoolNodeView class
//=============================================================
class SetBlackboardBoolNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SetBlackboardBoolNodeView(const std::string& key = "", bool value = false);
	~SetBlackboardBoolNodeView() override = default;

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

	std::string GetNodeType() const override { return "SET_BB_BOOL"; }
	void SaveParameters(BehaviorNodeData& data) const override;
	void LoadParameters(const BehaviorNodeData& data) override;

	//=========================================================
	// accsesser
	//=========================================================

	const std::string& GetBBKey() const { return bbKey_; }
	void SetBBKey(const std::string& key) { bbKey_ = key; }

	bool GetBBValue() const { return bbValue_; }
	void SetBBValue(bool value) { bbValue_ = value; }

private:
	std::string bbKey_;
	bool bbValue_;
};
