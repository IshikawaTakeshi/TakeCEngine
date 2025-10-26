#pragma once
#include "engine/Entity/GameCharacter.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include <optional>
#include <string>
#include <map>

// ビヘイビアの別名
using Behavior = GameCharacterBehavior;
// 前方宣言
class baseInputProvider; 

//============================================================================
// BaseBehavior class
//============================================================================
class BaseBehavior {
public:

	BaseBehavior() = default;
	virtual ~BaseBehavior() = default;

	// 初期化
	virtual void Initialize([[maybe_unused]]GameCharacterContext& characterInfo) = 0;
	// 更新
	virtual void Update(GameCharacterContext& characterInfo) = 0;
	// 遷移先のビヘイビアチェック
	std::pair<bool,Behavior> TransitionNextBehavior(Behavior nextBehavior);

	//----- getter ---------------------------

	// 遷移フラグ取得
	bool GetIsTransition() const { return isTransition_; }
	// 次のビヘイビア取得
	Behavior GetNextBehavior() const { return nextBehavior_; }

	//----- setter ---------------------------

	// 遷移フラグ設定
	void SetIsTransition(bool isTransition) { isTransition_ = isTransition; }
	// 次のビヘイビア設定
	void SetNextBehavior(Behavior nextBehavior) { nextBehavior_ = nextBehavior; }

protected:

	bool isTransition_ = false; // 遷移フラグ
	Behavior nextBehavior_ = Behavior::NONE; // 次のビヘイビア
	baseInputProvider* inputProvider_; // 移動方向を提供するインターフェース
	float deltaTime_ = 0.0f; // デルタタイム
};