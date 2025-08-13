#include "BehaviorManager.h"
#include "application/Entity/Behavior/BehaviorRunning.h"


void BehaviorManager::Initialize() {
	// 初期化時にデフォルトのビヘイビアを設定
	currentBehavior_ = std::make_unique<BehaviorRunning>(moveDirectionProvider_);
	currentBehavior_->Initialize(characterContext_);
}


