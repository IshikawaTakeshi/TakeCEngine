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
	BehaviorStatus Execute(PlayableCharacterInfo& characterInfo) override;

	void Reset() override;

private:

	GameCharacterState targetState_;
	GameCharacterStateManager* stateManager_;
	bool isStarted_ = false; // アクションが開始されたかどうか
};