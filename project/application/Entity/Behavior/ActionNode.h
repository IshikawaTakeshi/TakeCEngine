#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"
#include "application/Entity/State/GameCharacterState.h"
#include "application/Entity/State/GameCharacterStateManager.h"

//============================================================================
// ActionNode: 実際のステート遷移をリクエストし、完了を待つ
// → 既存のステートシステムとビヘイビアツリーの橋渡し
//============================================================================
class ActionNode : public BehaviorNode {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ActionNode(GameCharacterState targetState,
		GameCharacterStateManager* stateManager,
		const std::string& name = "Action");
	~ActionNode() override = default;

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

public:

	//=============================================================================
	// accessors
	//=============================================================================

	//---- getter ---------------------------

	/// 遷移先のステートを取得
	GameCharacterState GetTargetState() const { return targetState_; }
	/// 遷移先のステートを設定
	void SetTargetState(GameCharacterState state) { targetState_ = state; }

private:

	GameCharacterState targetState_;
	GameCharacterStateManager* stateManager_;
	bool isStarted_ = false; // アクションが開始されたかどうか
};