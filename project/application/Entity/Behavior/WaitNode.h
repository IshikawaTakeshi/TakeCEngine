#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"

//============================================================================
// WaitNode: 指定秒数だけ Running を返し続け、経過後 Success を返す
//============================================================================
class WaitNode : public BehaviorNode {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	WaitNode(float waitTime = 1.0f, const std::string& name = "Wait");
	~WaitNode() override = default;

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

	float GetWaitTime() const { return waitTime_; }
	void SetWaitTime(float t) { waitTime_ = t; }

private:

	float waitTime_    = 1.0f; // 待機時間 [s]
	float elapsedTime_ = 0.0f; // 累積経過時間 [s]
};
