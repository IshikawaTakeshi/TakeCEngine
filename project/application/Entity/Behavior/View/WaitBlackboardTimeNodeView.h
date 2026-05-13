#pragma once
#include "BehaviorNodeView.h"
#include <string>
#include <vector>

//=============================================================
// WaitBlackboardTimeNodeView class
// Blackboard の float キーをコンボで選択し、
// その値を待機時間として WaitBlackboardTimeNode に渡す View
//=============================================================
class WaitBlackboardTimeNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	WaitBlackboardTimeNodeView(const std::string& bbKey = "");
	~WaitBlackboardTimeNodeView() override = default;

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

	std::string GetNodeType() const override { return "WAIT_BB_TIME"; }
	void SaveParameters(BehaviorNodeData& data) const override;
	void LoadParameters(const BehaviorNodeData& data) override;

	//=========================================================
	// accessors
	//=========================================================

	const std::string& GetBBKey() const { return bbKey_; }
	void SetBBKey(const std::string& key) { bbKey_ = key; }

	/// <summary>
	/// Blackboard のキー名リストを外部から注入する（コンボ用）
	/// </summary>
	void SetBlackboardKeys(const std::vector<std::string>& keys) { blackboardKeys_ = keys; }

private:

	std::string bbKey_;                        // 選択中のキー名
	std::vector<std::string> blackboardKeys_;  // Blackboard から渡されるキー名リスト
};
