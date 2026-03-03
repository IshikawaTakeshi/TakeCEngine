#pragma once
#include "engine/Entity/GameCharacter.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include <optional>
#include <string>
#include <map>

// ビヘイビアの別名
using State = GameCharacterState;
// 前方宣言
class baseInputProvider; 

//============================================================================
// BaseGameCharacterState class
//============================================================================
class BaseGameCharacterState {
public:

	BaseGameCharacterState() = default;
	virtual ~BaseGameCharacterState() = default;

	// 初期化
	virtual void Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) = 0;
	// 更新
	virtual void Update(PlayableCharacterInfo& characterInfo) = 0;
	// 遷移先のビヘイビアチェック
	std::pair<bool,State> TransitionNextState(State nextState);

	//----- getter ---------------------------

	// 遷移フラグ取得
	bool GetIsTransition() const { return isTransition_; }
	// 次のビヘイビア取得
	State GetNextState() const { return nextState_; }

	//----- setter ---------------------------

	// 遷移フラグ設定
	void SetIsTransition(bool isTransition) { isTransition_ = isTransition; }
	// 次のビヘイビア設定
	void SetNextState(State nextState) { nextState_ = nextState; }

protected:

	bool isTransition_ = false; // 遷移フラグ
	State nextState_ = State::NONE; // 次のビヘイビア
	baseInputProvider* inputProvider_; // 移動方向を提供するインターフェース
	float deltaTime_ = 0.0f; // デルタタイム
};