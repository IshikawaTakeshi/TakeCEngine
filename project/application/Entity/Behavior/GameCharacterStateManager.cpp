#include "GameCharacterStateManager.h"
#include "application/Provider/BaseInputProvider.h"
#include "application/Entity/Behavior/StateRunning.h"
#include "application/Entity/Behavior/StateJumping.h"
#include "application/Entity/Behavior/StateFloating.h"
#include "application/Entity/Behavior/StateStepBoost.h"
#include "application/Entity/Behavior/StateChargeShootStun.h"
#include "application/Entity/Behavior/StateDead.h"
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

void GameCharacterStateManager::InitializeBehaviors(PlayableCharacterInfo& characterContext) {
	// ビヘイビアの初期化
	CreateDefaultBehaviors();
	// 初期化時にデフォルトのビヘイビアを設定
	currentBehavior_ = std::move(behaviors_[GameCharacterState::RUNNING]);
	currentBehavior_->Initialize(characterContext);
	currentBehaviorType_ = GameCharacterState::RUNNING;
}

//=====================================================================================
// 更新
//=====================================================================================

void GameCharacterStateManager::Update(PlayableCharacterInfo& characterContext) {
	// ビヘイビアの遷移チェック
	if (currentBehavior_) {
		if (currentBehavior_->GetIsTransition() == true) { // 遷移が必要な場合
			nextState_ = currentBehavior_->GetNextState();
			isChanged_ = true;
		}
	}

	if (isChanged_) {
		// ビヘイビアの遷移
		currentBehavior_->SetIsTransition(false); // 遷移フラグをリセット
		currentBehavior_->SetNextState(GameCharacterState::NONE); // 次のビヘイビアをリセット
		// 現在のビヘイビアを保存し、次のビヘイビアに切り替え
		behaviors_[currentBehaviorType_] = std::move(currentBehavior_);
		currentBehavior_ = std::move(behaviors_[nextState_]);
		currentBehaviorType_ = nextState_;
		currentBehavior_->Initialize(characterContext);

		// アニメーション遷移を開始
		if (animatorController_ && animationMapper_) {
			const AnimationEntry* entry = animationMapper_->Find(currentBehaviorType_);
			if (entry && entry->animation) {
				animatorController_->TransitionTo(entry->animation, entry->blendDuration);
			}
		}

		isChanged_ = false;
	}
	// 現在のビヘイビアを更新
	if (currentBehavior_) {
		currentBehavior_->Update(characterContext);
	}
}

//=====================================================================================
// ImGuiの更新
//=====================================================================================
void GameCharacterStateManager::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)

	//現在の項目
	std::string currentBehaviorStr = "Current State:" + StringUtility::EnumToString(currentBehaviorType_);

	ImGui::SeparatorText("GameCharacterStateManager");
	ImGui::Text(currentBehaviorStr.c_str());
#endif // _DEBUG
}

//=====================================================================================
// ビヘイビアのリクエスト
//=====================================================================================
void GameCharacterStateManager::RequestBehavior(State nextBehavior) {
	if (nextBehavior != GameCharacterState::NONE && nextBehavior != currentBehaviorType_) {
		nextState_ = nextBehavior;
		isChanged_ = true;
	}
}

//=====================================================================================
// 各ビヘイビアの取得
//=====================================================================================
void GameCharacterStateManager::CreateDefaultBehaviors() {
	behaviors_.emplace(GameCharacterState::RUNNING, std::make_unique<StateRunning>(inputProvider_));
	behaviors_.emplace(GameCharacterState::JUMP, std::make_unique<StateJumping>(inputProvider_));
	behaviors_.emplace(GameCharacterState::FLOATING, std::make_unique<StateFloating>(inputProvider_));
	behaviors_.emplace(GameCharacterState::STEPBOOST, std::make_unique<StateStepBoost>(inputProvider_));
	behaviors_.emplace(GameCharacterState::CHARGESHOOT_STUN, std::make_unique<StateChargeShootStun>(inputProvider_));
	behaviors_.emplace(GameCharacterState::DEAD, std::make_unique<StateDead>(inputProvider_));
}

//=====================================================================================
// アニメーションコンポーネントの設定
//=====================================================================================
void GameCharacterStateManager::SetAnimationComponents(AnimatorController* animatorController, AnimationMapper* animationMapper) {
	animatorController_ = animatorController;
	animationMapper_ = animationMapper;

	// 現在のビヘイビアに対応するアニメーションで初期遷移を行う
	if (animatorController_ && animationMapper_) {
		const AnimationEntry* entry = animationMapper_->Find(currentBehaviorType_);
		if (entry && entry->animation) {
			animatorController_->TransitionTo(entry->animation, 0.0f);
		}
	}
}