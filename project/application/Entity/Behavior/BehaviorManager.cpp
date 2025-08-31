#include "BehaviorManager.h"
#include "application/Provider/BaseInputProvider.h"
#include "application/Entity/Behavior/BehaviorRunning.h"
#include "application/Entity/Behavior/BehaviorJumping.h"
#include "application/Entity/Behavior/BehaviorFloating.h"
#include "application/Entity/Behavior/BehaviorStepBoost.h"
#include "application/Entity/Behavior/BehaviorChargeShootStun.h"
#include "application/Entity/Behavior/BehaviorDead.h"
#include "engine/base/TakeCFrameWork.h"

//=====================================================================================
// 初期化
//=====================================================================================

void BehaviorManager::Initialize(baseInputProvider* moveDirectionProvider) {
	inputProvider_ = moveDirectionProvider;
}

void BehaviorManager::InitializeBehaviors(GameCharacterContext& characterContext) {
	// ビヘイビアの初期化
	CreateDefaultBehaviors();
	// 初期化時にデフォルトのビヘイビアを設定
	currentBehavior_ = std::move(behaviors_[GameCharacterBehavior::RUNNING]);
	currentBehavior_->Initialize(characterContext);
	currentBehaviorType_ = GameCharacterBehavior::RUNNING;
}

//=====================================================================================
// 更新
//=====================================================================================

void BehaviorManager::Update(GameCharacterContext& characterContext) {
	// ビヘイビアの遷移チェック
	if (currentBehavior_) {
		if (currentBehavior_->GetIsTransition() == true) { // 遷移が必要な場合
			nextBehavior_ = currentBehavior_->GetNextBehavior();
			isChanged_ = true;
		}
	}

	if (isChanged_) {
		// ビヘイビアの遷移
		currentBehavior_->SetIsTransition(false); // 遷移フラグをリセット
		currentBehavior_->SetNextBehavior(GameCharacterBehavior::NONE); // 次のビヘイビアをリセット
		// 現在のビヘイビアを保存し、次のビヘイビアに切り替え
		behaviors_[currentBehaviorType_] = std::move(currentBehavior_);
		currentBehavior_ = std::move(behaviors_[nextBehavior_]);
		currentBehaviorType_ = nextBehavior_;
		currentBehavior_->Initialize(characterContext);
		isChanged_ = false;
	}
	// 現在のビヘイビアを更新
	if (currentBehavior_) {
		currentBehavior_->Update(characterContext);
	}
}

void BehaviorManager::UpdateImGui() {
#ifdef _DEBUG

	//コンボボックスの項目
	std::vector<std::string> items = {
		"IDLE","RUNNING", "JUMP","DASH",
		"CHARGESHOOT_STUN","HEAVYDAMAGE","STEPBOOST","FLOATING","DEAD"
	};
	//現在の項目
	std::string currentBehaviorStr = "Current Behavior:" + items[int(currentBehaviorType_)];

	ImGui::SeparatorText("BehaviorManager");
	ImGui::Text(currentBehaviorStr.c_str());
#endif // _DEBUG
}

//=====================================================================================
// ビヘイビアのリクエスト
//=====================================================================================

void BehaviorManager::RequestBehavior(Behavior nextBehavior) {
	if (nextBehavior != GameCharacterBehavior::NONE && nextBehavior != currentBehaviorType_) {
		nextBehavior_ = nextBehavior;
		isChanged_ = true;
	}
}

//=====================================================================================
// 各ビヘイビアの取得
//=====================================================================================

void BehaviorManager::CreateDefaultBehaviors() {
	behaviors_.emplace(GameCharacterBehavior::RUNNING, std::make_unique<BehaviorRunning>(inputProvider_));
	behaviors_.emplace(GameCharacterBehavior::JUMP, std::make_unique<BehaviorJumping>(inputProvider_));
	behaviors_.emplace(GameCharacterBehavior::FLOATING, std::make_unique<BehaviorFloating>(inputProvider_));
	behaviors_.emplace(GameCharacterBehavior::STEPBOOST, std::make_unique<BehaviorStepBoost>(inputProvider_));
	behaviors_.emplace(GameCharacterBehavior::CHARGESHOOT_STUN, std::make_unique<BehaviorChargeShootStun>(inputProvider_));
	behaviors_.emplace(GameCharacterBehavior::DEAD, std::make_unique<BehaviorDead>(inputProvider_));
}