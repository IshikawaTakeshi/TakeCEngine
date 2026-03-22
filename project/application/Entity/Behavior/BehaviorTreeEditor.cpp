#include "BehaviorTreeEditor.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/Utility/StringUtility.h"

using namespace TakeC;

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

		// Blackboardの表示
		if (ImGui::TreeNode("Blackboard")) {
			blackboard_->UpdateImGui();
			ImGui::TreePop();
		}
		ImGui::Separator();
		// コンボセットの保存ボタン
		SaveComboSet();

		//実行フローの強調表示（リンクのアニメーション管理）
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

						//Viewにnodeの状態を渡す（ノードのdraw内で状態に応じた色を表示するため）
						childView->SetCurrentStatus(node->GetCurrentStatus());
						
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
void BehaviorTreeEditor::LoadTreeFromEnemy(BehaviorNode* rootNode, Blackboard* blackboard) {
	if (!flowEditor_ || !rootNode) return;

	blackboard_ = blackboard;
	rootNode_ = rootNode;
	// いったんエディタのノードをクリアする
	nodeViewMap_.clear();
	ImVec2 startPos = ImVec2(100.0f, 100.0f);
	BuildNodeView(rootNode_, startPos);
}

//===============================================================================
// コンボセットのデータからツリーを構築して読み込む
//===============================================================================
void BehaviorTreeEditor::LoadTreeFromJson(const std::string&) {

	
}

//===============================================================================
// ツリーををコンボセットとしてファイルに保存する
//===============================================================================
void BehaviorTreeEditor::SaveComboSet() {
	if (!rootNode_) return;

	ComboSetData out;
	out.rootType = DetectRootType(rootNode_);

	// ルートは Composite 前提（ComboFactoryと同じ構造）
	auto* rootComposite = dynamic_cast<CompositeNode*>(rootNode_);
	if (!rootComposite) return;

	for (const auto& child : rootComposite->GetChildren()) {
		if (!child) continue;
		ComboData combo;
		combo.comboName = child->GetName(); // 1子 = 1コンボ名
		combo.rootNode = BuildNodeDataFromLogicNode(child.get());
		out.combos.push_back(std::move(combo));
	}	

	//ImGuiManagerの保存ポップアップを表示する
	ImGuiManager::ShowSavePopup(
		TakeCFrameWork::GetJsonLoader(),
		"Save Combo Set",
		out.setName.c_str(),
		out,
		out.setName);
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

	// ロジックノードとビューの紐づけ
	static_cast<BehaviorNodeView*>(viewNode)->SetLogicNode(node);

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

//===============================================================================
// ロジックノードから保存用のデータ構造を構築する
//===============================================================================
BehaviorNodeData BehaviorTreeEditor::BuildNodeDataFromLogicNode(const BehaviorNode* node) const {
	BehaviorNodeData data;
	if (!node) return data;

	data.name = node->GetName();

	if (auto* action = dynamic_cast<const ActionNode*>(node)) {
		data.nodeType = "ACTION";
		// ActionNodeから遷移先Stateを取得できるAPIが必要
		// 例: action->GetTargetState()
		data.targetState = StringUtility::EnumToString(action->GetTargetState());
	}
	else if (auto* cond = dynamic_cast<const ConditionNode*>(node)) {
		data.nodeType = "CONDITION";
		// ConditionNodeから保存用パラメータを取得できるAPIが必要
		// 例: cond->GetField(), cond->GetOperator(), cond->GetThreshold()
		data.field = cond->GetField();
		data.op = cond->GetOperator();
		data.conditionThreshold = cond->GetThreshold();
	}
	else if (auto* seq = dynamic_cast<const SequenceNode*>(node)) {
		data.nodeType = "SEQUENCE";
		for (const auto& c : seq->GetChildren()) {
			data.children.push_back(BuildNodeDataFromLogicNode(c.get()));
		}
	}
	else if (auto* sel = dynamic_cast<const SelectorNode*>(node)) {
		data.nodeType = "SELECTOR";
		for (const auto& c : sel->GetChildren()) {
			data.children.push_back(BuildNodeDataFromLogicNode(c.get()));
		}
	}
	else {
		// 未対応ノードは最低限ACTION/NONEで逃がすなど方針を決める
		data.nodeType = "ACTION";
		data.targetState = "NONE";
	}

	return data;
}

//===============================================================================
// ノードの種類を文字列で判別する（保存用）
//===============================================================================
std::string BehaviorTreeEditor::DetectRootType(const BehaviorNode* node) const {
	
	if (dynamic_cast<const ActionNode*>(node)) {
		return "ACTION";
	}
	else if (dynamic_cast<const ConditionNode*>(node)) {
		return "CONDITION";
	}
	else if (dynamic_cast<const ScoreConditionNode*>(node)) {
		return "SCORE_CONDITION";
	}
	else if (dynamic_cast<const SelectorNode*>(node)) {
		return "SELECTOR";
	}
	else if (dynamic_cast<const PlannerSelectorNode*>(node)) {
		return "PLANNER_SELECTOR";
	}
	else if (dynamic_cast<const SequenceNode*>(node)) {
		return "SEQUENCE";
	}
	else {
		return "UNKNOWN";
	}
}
