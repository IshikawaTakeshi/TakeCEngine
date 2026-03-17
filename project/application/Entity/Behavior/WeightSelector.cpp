#include "WeightSelector.h"

//==================================================================================
// ノードの実行
//==================================================================================
BehaviorStatus WeightSelectorNode::Execute(Blackboard& blackboard) {

	if (children_.empty()) {
		currentStatus_ = BehaviorStatus::Failure;
		return BehaviorStatus::Failure; // 子ノードがいない場合は失敗
	}
	size_t selectedIndex = SelectNodeIndex(blackboard);
	if (selectedIndex >= children_.size()) {
		currentStatus_ = BehaviorStatus::Failure;
		return BehaviorStatus::Failure; // 選択されたインデックスが範囲外の場合は失敗
	}

	// [EXT] 実行中のインデックスを保存 (CompositeNodeの機能を利用)
	currentIndex_ = selectedIndex;

	BehaviorStatus status = children_[selectedIndex]->Execute(blackboard);
	currentStatus_ = status;

	if (status == BehaviorStatus::Running) {
		return BehaviorStatus::Running; // 選択されたノードがまだ実行中
	}
	if (status == BehaviorStatus::Failure) {
		return BehaviorStatus::Failure; // 選択されたノードが失敗
	}
	return BehaviorStatus::Success; // 選択されたノードが成功
}

//==================================================================================
// 子ノードの評価値を計算して、評価値の分布からランダムにノードを選択する
//==================================================================================
size_t WeightSelectorNode::SelectNodeIndex(Blackboard&) {

	// 子ノードの評価値を計算
	std::vector<float> weights(children_.size());
	float totalWeight = 0.0f;
	for (size_t i = 0; i < children_.size(); i++) {
		// ここでは単純に1.0fを返すようにしているが、実際にはcharacterInfoを元に評価値を計算する
		weights[i] = 1.0f;
		totalWeight += weights[i];
	}
	// 評価値の分布からランダムにインデックスを選択
	float randomValue = static_cast<float>(rand()) / RAND_MAX * totalWeight;
	float cumulativeWeight = 0.0f;
	for (size_t i = 0; i < weights.size(); i++) {
		cumulativeWeight += weights[i];
		if (randomValue <= cumulativeWeight) {
			return i; // 選択されたインデックスを返す
		}
	}
	return weights.size() - 1; // 万が一のため、最後のインデックスを返す
}
