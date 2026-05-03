#pragma once
#include "BehaviorNodeView.h"
#include <string>

//=============================================================
// SetBlackboardStringNodeView class
//=============================================================
class SetBlackboardStringNodeView : public BehaviorNodeView {
public:

	/**
	 * @brief コンストラクタ。
	 * @param key  BBキーの初期値
	 * @param value セットする文字列の初期値
	 */
	SetBlackboardStringNodeView(const std::string& key = "", const std::string& value = "");
	~SetBlackboardStringNodeView() override = default;

	//=========================================================
	// functions
	//=========================================================

	/**
	 * @brief ノードの内容をImGuiで描画する。
	 */
	void draw() override;

	//=========================================================
	// serialization [EXT]
	//=========================================================

	std::string GetNodeType() const override { return "SET_BB_STRING"; }
	void SaveParameters(BehaviorNodeData& data) const override;
	void LoadParameters(const BehaviorNodeData& data) override;

	//=========================================================
	// accessors
	//=========================================================

	const std::string& GetBBKey() const { return bbKey_; }
	void SetBBKey(const std::string& key) { bbKey_ = key; }

	const std::string& GetBBStringValue() const { return bbStringValue_; }
	void SetBBStringValue(const std::string& value) { bbStringValue_ = value; }

private:
	// 書き込み先のBlackboardキー
	std::string bbKey_;
	// セットする文字列値
	std::string bbStringValue_;
};
