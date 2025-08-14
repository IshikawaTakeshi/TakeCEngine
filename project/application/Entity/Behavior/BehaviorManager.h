#pragma once
#include "application/Entity/Behavior/BaseBehavior.h"
#include <memory>
#include <unordered_map>

class IMoveDirectionProvider;
class BehaviorManager {
public:
	BehaviorManager() = default;
	~BehaviorManager() = default;
	// ビヘイビアの初期化
	void Initialize(IMoveDirectionProvider* moveDirectionProvider);
	void InitializeBehaviors(GameCharacterContext& characterContext);
	// ビヘイビアの更新
	void Update(GameCharacterContext& characterContext);
	void UpdateImGui();
	// ビヘイビアの遷移リクエスト
	void RequestBehavior(Behavior nextBehavior);

	Behavior GetCurrentBehaviorType() const { return currentBehaviorType_; }
	Behavior GetNextBehaviorType() const { return nextBehavior_; }

private:

	// 遷移先のビヘイビアを生成する
	void CreateDefaultBehaviors();

private:

	std::unique_ptr<BaseBehavior> currentBehavior_; // 現在のビヘイビア
	std::unordered_map<Behavior, std::unique_ptr<BaseBehavior>> behaviors_; // ビヘイビアのマップ

	Behavior currentBehaviorType_ = Behavior::NONE; // 現在のビヘイビアタイプ
	Behavior nextBehavior_ = Behavior::NONE; // 次のビヘイビア
	bool isChanged_ = false; // ビヘイビアが変更されたかどうか

	IMoveDirectionProvider* moveDirectionProvider_ = nullptr;
};

