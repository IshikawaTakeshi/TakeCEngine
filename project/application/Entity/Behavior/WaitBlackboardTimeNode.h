#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"
#include <string>

//============================================================================
// WaitBlackboardTimeNode
// Blackboard の float キーを待機秒数として使い、経過後 Success を返す
//============================================================================
class WaitBlackboardTimeNode : public BehaviorNode {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	WaitBlackboardTimeNode(const std::string& bbKey = "", const std::string& name = "WaitBBTime");
	~WaitBlackboardTimeNode() override = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// ノードの実行
	/// </summary>
	BehaviorStatus Execute(Blackboard& blackboard) override;

	/// <summary>
	/// ノードのリセット（再実行時に状態をクリア）
	/// </summary>
	void Reset() override;

	/// <summary>
	/// ImGuiを用いたデバッグ用情報の描画
	/// </summary>
	void DrawInspector() override;

	//========================================================================
	// accessors
	//========================================================================

	const std::string& GetBBKey() const { return bbKey_; }
	void SetBBKey(const std::string& key) { bbKey_ = key; }

private:

	std::string bbKey_;             // Blackboard から待機秒数を読むキー名
	float elapsedTime_    = 0.0f;  // 累積経過時間 [s]
	float cachedWaitTime_ = 0.0f;  // 初回 Execute 時にキャッシュした待機時間
	bool  isCached_       = false; // キャッシュ済みフラグ
};
