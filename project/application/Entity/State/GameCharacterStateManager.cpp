#include "GameCharacterStateManager.h"
#include "application/Provider/BaseInputProvider.h"
#include "application/Entity/State/StateRunning.h"
#include "application/Entity/State/StateJumping.h"
#include "application/Entity/State/StateFloating.h"
#include "application/Entity/State/StateStepBoost.h"
#include "application/Entity/State/StateChargeShootStun.h"
#include "application/Entity/State/StateDead.h"
#include "application/Entity/State/StateBreakStun.h"
#include "application/Entity/Animation/AnimationMapper.h"
#include "engine/Animation/AnimatorController.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/Utility/StringUtility.h"

//=====================================================================================
// 初期化
//=====================================================================================

void GameCharacterStateManager::Initialize(baseInputProvider* moveDirectionProvider) {
	inputProvider_ = moveDirectionProvider; //入力プロバイダーの設定
}

void GameCharacterStateManager::InitializeStates(PlayableCharacterInfo& characterContext) {
	// ステートの初期化
	CreateDefaultStates();
	// 初期化時にデフォルトのステートを設定
	currentState_ = std::move(states_[GameCharacterState::RUNNING]);
	currentState_->Initialize(characterContext);
	currentStateType_ = GameCharacterState::RUNNING;
}

//=====================================================================================
// 更新
//=====================================================================================

void GameCharacterStateManager::Update(PlayableCharacterInfo& characterContext) {
	// ステートの遷移チェック
	if (currentState_) {
		if (currentState_->GetIsTransition() == true) { // 遷移が必要な場合
			nextState_ = currentState_->GetNextState();
			isChanged_ = true;
		}
	}

	if (isChanged_) {
		// ステートの遷移
		currentState_->SetIsTransition(false); // 遷移フラグをリセット
		currentState_->SetNextState(GameCharacterState::NONE); // 次のステートをリセット
		// 現在のステートを保存し、次のステートに切り替え
		states_[currentStateType_] = std::move(currentState_);
		currentState_ = std::move(states_[nextState_]);
		currentStateType_ = nextState_;
		currentState_->Initialize(characterContext);

		// アニメーション遷移を開始
		if (animatorController_ && animationMapper_) {
			const AnimationEntry* entry = animationMapper_->Find(currentStateType_);
			if (entry && entry->animation) {
				animatorController_->TransitionTo(entry->animation, entry->blendDuration);
			}
		}

		isChanged_ = false;
	}
	// 現在のステートを更新
	if (currentState_) {
		currentState_->Update(characterContext);
	}
}

//=====================================================================================
// ImGuiの更新
//=====================================================================================
void GameCharacterStateManager::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)

	//現在の項目
	std::string currentBehaviorStr = "Current State:" + StringUtility::EnumToString(currentStateType_);

	ImGui::SeparatorText("GameCharacterStateManager");
	ImGui::Text(currentBehaviorStr.c_str());
#endif // _DEBUG
}

//=====================================================================================
// ステートのリクエスト
//=====================================================================================
void GameCharacterStateManager::RequestState(State nextBehavior) {
	if (nextBehavior != GameCharacterState::NONE && nextBehavior != currentStateType_) {
		nextState_ = nextBehavior;
		isChanged_ = true;
	}
}

//=====================================================================================
// 各ステートの取得
//=====================================================================================
void GameCharacterStateManager::CreateDefaultStates() {
	states_.emplace(GameCharacterState::RUNNING, std::make_unique<StateRunning>(inputProvider_));
	states_.emplace(GameCharacterState::JUMP, std::make_unique<StateJumping>(inputProvider_));
	states_.emplace(GameCharacterState::FLOATING, std::make_unique<StateFloating>(inputProvider_));
	states_.emplace(GameCharacterState::STEPBOOST, std::make_unique<StateStepBoost>(inputProvider_));
	states_.emplace(GameCharacterState::CHARGESHOOT_STUN, std::make_unique<StateChargeShootStun>(inputProvider_));
	states_.emplace(GameCharacterState::DEAD, std::make_unique<StateDead>(inputProvider_));
	states_.emplace(GameCharacterState::BREAK_STUN, std::make_unique<StateBreakStun>(inputProvider_));
}

//=====================================================================================
// アニメーションコンポーネントの設定
//=====================================================================================
void GameCharacterStateManager::SetAnimationComponents(AnimatorController* animatorController, AnimationMapper* animationMapper) {
	animatorController_ = animatorController;
	animationMapper_ = animationMapper;

	// 現在のステートに対応するアニメーションで初期遷移を行う
	if (animatorController_ && animationMapper_) {
		const AnimationEntry* entry = animationMapper_->Find(currentStateType_);
		if (entry && entry->animation) {
			animatorController_->TransitionTo(entry->animation, 0.0f);
		}
	}
}