#include "BehaviorTreeEditor.h"
#include "engine/Base/ImGuiManager.h"

//==================================================================================
// 初期化
//==================================================================================
void BehaviorTreeEditor::Initialize() {

	// ImNodeFlowエディタの初期化
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");

	//右クリックメニューのノードを追加可能にする
	flowEditor_->rightClickPopUpContent([this](ImFlow::BaseNode*) {
		if (ImGui::MenuItem("Add Action")) { flowEditor_->placeNode<ActionNodeView>("NONE"); }
		if (ImGui::MenuItem("Add Condition")) { flowEditor_->placeNode<ConditionNodeView>(); }
		if (ImGui::MenuItem("Add ScoreCondition")) { flowEditor_->placeNode<ScoreConditionNodeView>(); }
		if (ImGui::MenuItem("Add Selector")) { flowEditor_->placeNode<SelectorNodeView>(); }
		if (ImGui::MenuItem("Add Sequence")) { flowEditor_->placeNode<SequenceNodeView>(); }
		if (ImGui::MenuItem("Add PlannerSelector")) { flowEditor_->placeNode<PlannerSelectorNodeView>(); }
		if (ImGui::MenuItem("Add WeightSelector")) { flowEditor_->placeNode<WeightSelectorNodeView>(); }

		});
}

//===============================================================================
// 描画
//===============================================================================
void BehaviorTreeEditor::UpdateImGui() {
	if (flowEditor_) {
		ImGui::Begin("Behavior Tree Editor");

		// [EXT] 実行フローの強調表示（リンクのアニメーション管理）
		for (auto& link_weak : flowEditor_->getLinks()) {
			if (auto link = link_weak.lock()) {
				// いったん非アクティブにリセット
				link->setActive(false);

				// リンクの右側（入力ピン）に繋がっているノードを取得
				auto* childView = static_cast<BehaviorNodeView*>(link->right()->getParent());
				
				// Viewに対応する BehaviorNode を逆引き
				for (auto& pair : nodeViewMap_) {
					if (pair.second == childView) {
						BehaviorNode* node = pair.first;
						
						// ノードが実行中 (Running) なら、その入力リンクを光らせる
						if (node->GetCurrentStatus() == BehaviorStatus::Running) {
							link->setActive(true);
						}

						// 親ノードが CompositeNode の場合、現在アクティブなインデックスを確認
						auto* parentView = static_cast<BehaviorNodeView*>(link->left()->getParent());
						for (auto& parentPair : nodeViewMap_) {
							if (parentPair.second == parentView) {
								if (auto* composite = dynamic_cast<CompositeNode*>(parentPair.first)) {
									const auto& children = composite->GetChildren();
									size_t activeIdx = composite->GetCurrentIndex();
									if (activeIdx < children.size() && children[activeIdx].get() == node) {
										if (composite->GetCurrentStatus() == BehaviorStatus::Running) {
											link->setActive(true);
										}
									}
								}
								break;
							}
						}
						break;
					}
				}
			}
		}

		flowEditor_->update();
		ImGui::End();
	}
}

//===============================================================================
// Enemyからツリーを読み込む
//===============================================================================
void BehaviorTreeEditor::LoadTreeFromEnemy(BehaviorNode* rootNode) {
	if (!flowEditor_ || !rootNode) return;

	// いったんエディタのノードをクリアする
	nodeViewMap_.clear();
	ImVec2 startPos = ImVec2(100.0f, 100.0f);
	BuildNodeView(rootNode, startPos);
}

//===============================================================================
// 再帰的にノードビューを構築する
//===============================================================================
ImFlow::BaseNode* BehaviorTreeEditor::BuildNodeView(BehaviorNode* node, ImVec2& minPos) {
	if (!node || !flowEditor_) return nullptr;

	ImFlow::BaseNode* viewNode = nullptr;

	// 1. ノードクラスに合わせてViewを生成
	if (auto* actionNode = dynamic_cast<ActionNode*>(node)) {
		auto v = flowEditor_->placeNode<ActionNodeView>(node->GetName());
		v->setPos(minPos);
		viewNode = v.get();
	}
	else if (auto* conditionNode = dynamic_cast<ConditionNode*>(node)) {
		auto v = flowEditor_->placeNode<ConditionNodeView>();
		v->setPos(minPos);
		viewNode = v.get();
	}
	else if (auto* scoreConditionNode = dynamic_cast<ScoreConditionNode*>(node)) {
		auto v = flowEditor_->placeNode<ScoreConditionNodeView>();
		v->setPos(minPos);
		viewNode = v.get();
	}
	else if (auto* selectorNode = dynamic_cast<SelectorNode*>(node)) {
		auto v = flowEditor_->placeNode<SelectorNodeView>();
		v->setPos(minPos);
		viewNode = v.get();
	}
	else if (auto* plannerSelectorNode = dynamic_cast<PlannerSelectorNode*>(node)) {
		auto v = flowEditor_->placeNode<PlannerSelectorNodeView>();
		v->setPos(minPos);
		viewNode = v.get();
	}
	else if (auto* sequenceNode = dynamic_cast<SequenceNode*>(node)) {
		auto v = flowEditor_->placeNode<SequenceNodeView>();
		v->setPos(minPos);
		viewNode = v.get();
	}

	if (!viewNode) {
		// 未知のノード
		return nullptr;
	}

	// 紐づけを保存
	nodeViewMap_[node] = viewNode;

	// 2. CompositeNodeの場合は子ノードを再帰的に生成してリンクを繋ぐ
	if (auto* compositeNode = dynamic_cast<CompositeNode*>(node)) {

		const auto& children = compositeNode->GetChildren();
		ImVec2 childPos = ImVec2(minPos.x + 300.0f, minPos.y);

		for (size_t i = 0; i < children.size(); ++i) {
			ImFlow::BaseNode* childView = BuildNodeView(children[i].get(), childPos);

			if (childView) {
				// BehaviorNodeViewにキャストして保持しているピンを取得する
				auto* behaviorViewNode = static_cast<BehaviorNodeView*>(viewNode);
				
				// 子ノードの数に対してピンが足りない場合は動的に追加する
				// (初期状態で1つあるが、2つ目以降が必要な場合)
				while (behaviorViewNode->GetOutputPinCount() <= i) {
					std::string name = "Child" + std::to_string(behaviorViewNode->GetOutputPinCount());
					auto out = behaviorViewNode->addOUT<BehaviorStatus>(name);
					out->behaviour([]() { return BehaviorStatus::Invalid; });
					behaviorViewNode->AddOutputPin(out.get());
				}

				ImFlow::Pin* outPinPtr = behaviorViewNode->GetOutputPin(i); // 親ノードのi番目の出力ピン
				ImFlow::Pin* inPinPtr = static_cast<BehaviorNodeView*>(childView)->GetInputPin(0); // 子ノードの最初の入力ピン

				// Linkオブジェクトを作成してaddLinkに渡す
				if (outPinPtr && inPinPtr) {
					// ライブラリ標準の接続処理を呼び出す
					inPinPtr->createLink(outPinPtr);
				}
			}
			childPos.y += 100.0f; // 次の子ノードはY軸を少し下にズラす
		}

		// 親のY座標更新（兄弟ノードと被らないように）
		minPos.y = childPos.y;
	}

	return viewNode;
}
