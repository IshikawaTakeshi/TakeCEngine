#include "BehaviorTreeEditor.h"
// Logic
#include "application/Entity/Behavior/BehaviorNode.h"
#include "application/Entity/Behavior/Blackboard.h"
#include "application/Entity/Behavior/ComboFactory.h"
#include "application/Entity/Behavior/CompositeNode.h"
#include "application/Entity/Behavior/SelectorNode.h"
#include "application/Entity/Behavior/SequenceNode.h"
#include "application/Entity/Behavior/ConditionNode.h"
#include "application/Entity/Behavior/ScoreConditionNode.h"
#include "application/Entity/Behavior/PlannerSelectorNode.h"
#include "application/Entity/Behavior/ActionNode.h"
#include "application/Entity/Behavior/WeightSelectorNode.h"
#include "application/Entity/Behavior/SetBlackboardBoolNode.h"

// Views
#include "application/Entity/Behavior/View/ActionNodeView.h"
#include "application/Entity/Behavior/View/SelectorNodeView.h"
#include "application/Entity/Behavior/View/ConditionNodeView.h"
#include "application/Entity/Behavior/View/SequenceNodeView.h"
#include "application/Entity/Behavior/View/ScoreConditionNodeView.h"
#include "application/Entity/Behavior/View/PlannerSelectorNodeView.h"
#include "application/Entity/Behavior/View/WeightSelectorNodeView.h"
#include "application/Entity/Behavior/View/SetBlackboardBoolNodeView.h"

#include "engine/Base/ImGuiManager.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Utility/StringUtility.h"

using namespace TakeC;

BehaviorTreeEditor::BehaviorTreeEditor() = default;
BehaviorTreeEditor::~BehaviorTreeEditor() = default;

//==================================================================================
// 初期化
//==================================================================================
void BehaviorTreeEditor::Initialize() {

	// ImNodeFlowエディタの初期化
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");

	//右クリックメニューのノードを追加可能にする
	SetupContextMenu();

	//コンボセットのリストを取得
	comboSetNames_ = TakeCFrameWork::GetJsonLoader()->GetJsonDataList<ComboSetData>();
	
}

//==================================================================================
// ロジックツリーを再帰的に再構築する
//==================================================================================
std::unique_ptr<BehaviorNode> BehaviorTreeEditor::BuildLogicTree(const BehaviorNodeData& data, std::map<int, std::shared_ptr<BehaviorNodeView>>& idToNode) {
	std::unique_ptr<BehaviorNode> node = nullptr;

	// 1. タイプに応じてロジックノードをインスタンス化
	if (data.nodeType == "ACTION") {
		GameCharacterState state = StringUtility::StringToEnum<GameCharacterState>(data.targetState);
		node = std::make_unique<ActionNode>(state, nullptr, data.name);
	} else if (data.nodeType == "SET_BB_BOOL") {
		node = std::make_unique<SetBlackboardBoolNode>(data.bbKey, data.bbValue, data.name);
	} else if (data.nodeType == "CONDITION") {
		node = std::make_unique<ConditionNode>(data.field, data.op, data.conditionThreshold, data.name);
	} else if (data.nodeType == "SCORE_CONDITION") {
		// エディタ用なので、ダミーのスコア関数を渡す
		node = std::make_unique<ScoreConditionNode>([]() { return 0.0f; }, data.conditionThreshold, data.name);
	} else if (data.nodeType == "SELECTOR") {
		node = std::make_unique<SelectorNode>();
	} else if (data.nodeType == "SEQUENCE") {
		node = std::make_unique<SequenceNode>();
	} else if (data.nodeType == "PLANNER_SELECTOR") {
		node = std::make_unique<PlannerSelectorNode>();
	} else if (data.nodeType == "WEIGHT_SELECTOR") {
		node = std::make_unique<WeightSelectorNode>();
	}

	if (!node) return nullptr;
	node->SetName(data.name);
	node->SetUID(data.nodeUID);

	// 2. idToNode マップを使用して UI ノードと紐付ける
	auto it = idToNode.find(data.nodeUID);
	if (it != idToNode.end()) {
		auto view = it->second;
		nodeViewMap_[node.get()] = view.get(); // 逆引きマップ
		view->SetLogicNode(node.get());        // UI側からの参照
	}

	// 3. CompositeNode の場合は子ノードを再帰的に構築
	if (auto* composite = dynamic_cast<CompositeNode*>(node.get())) {
		for (const auto& childData : data.children) {
			auto childNode = BuildLogicTree(childData, idToNode);
			if (childNode) {
				composite->AddChild(std::move(childNode));
			}
		}
	}

	return node;
}


//================================================================================
// 実行中のロジックツリーとUIを再紐付けする
//================================================================================
void BehaviorTreeEditor::SyncWithActiveTree(BehaviorNode* root) {
	if (!flowEditor_ || !root) return;

	// 1. 全ての UI ノード (BehaviorNodeView) を UID をキーにして収集
	std::map<int, BehaviorNodeView*> uidToView;
	auto nodes = flowEditor_->getNodes();
	for (auto& node : nodes) {
		if (auto* view = dynamic_cast<BehaviorNodeView*>(node.second.get())) {
			uidToView[view->GetNodeUID()] = view; // JSON由来のUIDで紐付け

			//新しいツリーと同期する前に、古いポインタを確実にリセットする
			view->SetLogicNode(nullptr);
		}
	}

	// 2. マップをクリアして再構築
	nodeViewMap_.clear();

	// 3. ロジックツリーを走査して紐付け
	std::function<void(BehaviorNode*)> traverse = [&](BehaviorNode* node) {
		if (!node) return;

		auto it = uidToView.find(node->GetUID());
		if (it != uidToView.end()) {
			nodeViewMap_[node] = it->second;
			it->second->SetLogicNode(node);
		}

		if (auto* composite = dynamic_cast<CompositeNode*>(node)) {
			for (auto& child : composite->GetChildren()) {
				traverse(child.get());
			}
		}
		};

	traverse(root);
	rootNode_ = root;
}

//===============================================================================
// 描画
//===============================================================================
void BehaviorTreeEditor::UpdateImGui(BehaviorNode* activeRoot) {
	// 外部からアクティブなルートが渡された場合、現在の監視対象と違えば同期する
	if (activeRoot && activeRoot != rootNode_) {
		SyncWithActiveTree(activeRoot);
	}

	if (flowEditor_) {
		ImGui::Begin("Behavior Tree Editor");

		// Blackboardの表示
		if (ImGui::TreeNode("Blackboard")) {
			if (blackboard_) {
				blackboard_->UpdateImGui();
			}
			ImGui::TreePop();
		}
		ImGui::Separator();
		// コンボセットの保存ボタン
		SaveComboSet();

		// エネミーへの反映ボタン

		if (ImGui::Button("Apply Behavior Tree")) {
			currentComboSetData_ = BuildComboSetDataFromEditor();
			if (applyCallback_) {
				applyCallback_(currentComboSetData_);
			}
		}

		//コンボセットの読み込みボタン
		if (ImGui::CollapsingHeader("Load ComboSet")) {
			int currentPresetIndex_ = -1;
			bool isSelected = false;
			isSelected = ImGuiManager::ComboBoxString("ComboSet", comboSetNames_, currentPresetIndex_);

			if (isSelected) {
				//選択されたプリセットのツリーを読み込む
				LoadTreeFromJson(comboSetNames_[currentPresetIndex_] + ".json");
			}
		}

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
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");
	// 右クリックメニューを再設定
	SetupContextMenu();

	// ルートのCompositeNodeは「外側のwrapper」として扱い、
	// その型をrootTypeに記録した上で、子ノード群だけをエディタに展開する。
	// こうすることでBuildComboSetDataFromEditor()が各子を
	// 独立したコンボとして正しく収集できる。
	auto* composite = dynamic_cast<CompositeNode*>(rootNode);
	if (composite) {
		currentComboSetData_.rootType = DetectRootType(composite);
		currentComboSetData_.setName = composite->GetName();

		ImVec2 startPos = ImVec2(100.0f, 100.0f);
		for (auto& child : composite->GetChildren()) {
			BuildNodeView(child.get(), startPos);
			startPos.y += 200.0f; // 兄弟コンボ間のY間隔
		}
	} else {
		// CompositeNodeでない（葉ノード単体）場合はそのまま展開
		currentComboSetData_.rootType = "SELECTOR";
		ImVec2 startPos = ImVec2(100.0f, 100.0f);
		BuildNodeView(rootNode_, startPos);
	}

	currentComboSetData_ = BuildComboSetDataFromEditor();
}

//===============================================================================
// コンボセットのデータからツリーを構築して読み込む
//===============================================================================
void BehaviorTreeEditor::LoadTreeFromJson(const std::string& filepath) {
	if (!flowEditor_) return;

	// ファイルが存在するか確認
	if (!TakeCFrameWork::GetJsonLoader()->IsJsonDataExists<ComboSetData>(filepath)) {
		return;
	}

	// データの読み込み（JsonLoader::LoadJsonData は T を返す）
	currentComboSetData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<ComboSetData>(filepath);

	// FIX: flowEditor_を再作成する前にマップをクリア
	nodeViewMap_.clear();
	rootNode_ = nullptr;  // 古い参照も無効化

	// エディタをクリア
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");
	// 右クリックメニューを再設定
	SetupContextMenu();

	std::map<int, std::shared_ptr<BehaviorNodeView>> idToNode;

	// 1. ノードの生成
	for (const auto& nodeData : currentComboSetData_.editorNodes) {
		ImVec2 pos = ImVec2(nodeData.posX, nodeData.posY);
		auto v = CreateNodeView(nodeData.nodeType, pos, nodeData.name);

		if (v) {
			v->LoadParameters(nodeData);
			v->SetNodeUID(nodeData.nodeUID); // 同期に使う JSON UID をセット
			idToNode[nodeData.nodeUID] = v;
		}
	}

	// 2. リンクの構築
	for (const BehaviorLinkData& linkData : currentComboSetData_.editorLinks) {
		std::map<int, std::shared_ptr<BehaviorNodeView>>::iterator itFrom = idToNode.find(linkData.fromNodeUID);
		std::map<int, std::shared_ptr<BehaviorNodeView>>::iterator itTo = idToNode.find(linkData.toNodeUID);

		if (itFrom != idToNode.end() && itTo != idToNode.end()) {
			std::shared_ptr<BehaviorNodeView> fromNode = itFrom->second;
			std::shared_ptr<BehaviorNodeView> toNode = itTo->second;

			// 出力ピンが足りない場合は追加する（Selector/Sequence用）
			while (fromNode->GetOutputPinCount() <= (size_t)linkData.fromPinIndex) {
				std::string name = "Child" + std::to_string(fromNode->GetOutputPinCount());
				std::shared_ptr<ImFlow::OutPin<BehaviorStatus>> out = fromNode->addOUT<BehaviorStatus>(name);
				out->behaviour([]() { return BehaviorStatus::Invalid; });
				fromNode->AddOutputPin(out.get());
			}

			ImFlow::Pin* outPin = fromNode->GetOutputPin(linkData.fromPinIndex);
			ImFlow::Pin* inPin = toNode->GetInputPin(linkData.toPinIndex);

			if (outPin && inPin) {
				inPin->createLink(outPin);
			}
		}
	}

	// 3. ロジックノードの再構築
	std::unique_ptr<CompositeNode> rootLogicNode = nullptr;
	if (currentComboSetData_.rootType == "SEQUENCE") {
		rootLogicNode = std::make_unique<SequenceNode>();
	} else if (currentComboSetData_.rootType == "SELECTOR") {
		rootLogicNode = std::make_unique<SelectorNode>();
	} else if (currentComboSetData_.rootType == "PLANNER_SELECTOR") {
		rootLogicNode = std::make_unique<PlannerSelectorNode>();
	} else if (currentComboSetData_.rootType == "WEIGHT_SELECTOR") {
		rootLogicNode = std::make_unique<WeightSelectorNode>();
	} else {
		// デフォルトはセレクター
		rootLogicNode = std::make_unique<SelectorNode>();
	}
	rootLogicNode->SetName(currentComboSetData_.setName);

	for (const auto& combo : currentComboSetData_.combos) {
		auto comboNode = BuildLogicTree(combo.rootNode, idToNode);
		if (comboNode) {
			comboNode->SetName(combo.comboName);
			rootLogicNode->AddChild(std::move(comboNode));
		}
	}

	// 4. エディタの状態を更新
	ownedRootNode_ = std::move(rootLogicNode);
	rootNode_ = ownedRootNode_.get();
}

//===============================================================================
// ツリーををコンボセットとしてファイルに保存する
//===============================================================================
void BehaviorTreeEditor::SaveComboSet() {
	if (!flowEditor_) return;

	// ImGuiManagerの保存ポップアップを表示する
	ImGuiManager::ShowSavePopup(
		TakeCFrameWork::GetJsonLoader(),
		"Save Combo Set",
		std::string(currentComboSetData_.setName + ".json").c_str(),
		currentComboSetData_,
		currentComboSetData_.setName);

}

//===============================================================================
// 再帰的にノードビューを構築する
//===============================================================================
ImFlow::BaseNode* BehaviorTreeEditor::BuildNodeView(BehaviorNode* node, ImVec2& minPos) {
	if (!node || !flowEditor_) return nullptr;

	ImFlow::BaseNode* viewNode = nullptr;

	// 1. ノードクラスに合わせてViewを生成
	std::string type = DetectRootType(node);
	auto v = CreateNodeView(type, minPos, node->GetName());

	if (!v) {
		// 未知のノード
		return nullptr;
	}
	viewNode = v.get();

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
// ノードビューを生成する (内部用)
//===============================================================================
std::shared_ptr<BehaviorNodeView> BehaviorTreeEditor::CreateNodeView(const std::string& type, const ImVec2& pos, const std::string& name) {
	if (type == "ACTION") return flowEditor_->addNode<ActionNodeView>(pos, name);
	if (type == "SET_BB_BOOL") return flowEditor_->addNode<SetBlackboardBoolNodeView>(pos);
	if (type == "CONDITION") return flowEditor_->addNode<ConditionNodeView>(pos);
	if (type == "SCORE_CONDITION") return flowEditor_->addNode<ScoreConditionNodeView>(pos);
	if (type == "SELECTOR") return flowEditor_->addNode<SelectorNodeView>(pos);
	if (type == "SEQUENCE") return flowEditor_->addNode<SequenceNodeView>(pos);
	if (type == "PLANNER_SELECTOR") return flowEditor_->addNode<PlannerSelectorNodeView>(pos);
	if (type == "WEIGHT_SELECTOR") return flowEditor_->addNode<WeightSelectorNodeView>(pos);
	return nullptr;
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
	} else if (auto* cond = dynamic_cast<const ConditionNode*>(node)) {
		data.nodeType = "CONDITION";
		// ConditionNodeから保存用パラメータを取得できるAPIが必要
		// 例: cond->GetField(), cond->GetOperator(), cond->GetThreshold()
		data.field = cond->GetField();
		data.op = cond->GetOperator();
		data.conditionThreshold = cond->GetThreshold();
	} else if (auto* seq = dynamic_cast<const SequenceNode*>(node)) {
		data.nodeType = "SEQUENCE";
		for (const auto& c : seq->GetChildren()) {
			data.children.push_back(BuildNodeDataFromLogicNode(c.get()));
		}
	} else if (auto* sel = dynamic_cast<const SelectorNode*>(node)) {
		data.nodeType = "SELECTOR";
		for (const auto& c : sel->GetChildren()) {
			data.children.push_back(BuildNodeDataFromLogicNode(c.get()));
		}
	} else {
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
	} else if (dynamic_cast<const SetBlackboardBoolNode*>(node)) {
		return "SET_BB_BOOL";
	} else if (dynamic_cast<const ConditionNode*>(node)) {
		return "CONDITION";
	} else if (dynamic_cast<const ScoreConditionNode*>(node)) {
		return "SCORE_CONDITION";
	} else if (dynamic_cast<const SelectorNode*>(node)) {
		return "SELECTOR";
	} else if (dynamic_cast<const PlannerSelectorNode*>(node)) {
		return "PLANNER_SELECTOR";
	} else if (dynamic_cast<const SequenceNode*>(node)) {
		return "SEQUENCE";
	} else {
		return "UNKNOWN";
	}
}

//===============================================================================
// エディタ上のノード接続（ImNodeFlow）から ComboSetData を構成する
//===============================================================================
ComboSetData BehaviorTreeEditor::BuildComboSetDataFromEditor() const {
	ComboSetData out;
	if (!flowEditor_) return out;

	const auto& allLinks = flowEditor_->getLinks();

	// --- 1. ルートノード（入力接続がないノード）を特定し、再帰的に構造を構築 ---
	auto& nodes = flowEditor_->getNodes();
	for (auto& pair : nodes) {
		auto* viewNode = static_cast<BehaviorNodeView*>(pair.second.get());
		if (!viewNode) continue;

		// 全ての入力ピンにリンクがないかチェック
		bool hasInLink = false;
		for (auto& inPinSh : viewNode->getIns()) {
			ImFlow::Pin* targetPin = inPinSh.get();
			for (auto& link_weak : allLinks) {
				std::shared_ptr<ImFlow::Link> link = link_weak.lock();
				if (link && link->right() == targetPin) {
					hasInLink = true;
					break;
				}
			}
			if (hasInLink) break;
		}

		// 入力がない = このツリーの(サブ)ルート
		if (!hasInLink) {
			ComboData combo;
			combo.comboName = viewNode->getName();
			combo.rootNode = BuildRecursiveNodeData(viewNode);
			out.combos.push_back(combo);
		}
	}

	// 抽出したRootサブツリーの順番もY座標でソートしておく
	std::sort(out.combos.begin(), out.combos.end(), [](const ComboData& a, const ComboData& b) {
		return a.rootNode.posY < b.rootNode.posY;
		});

	// --- 2. エディタレイアウト情報（フラットなリスト）を保存 ---
	std::map<ImFlow::BaseNode*, int> nodePtrToUid;

	// 既存UIDの最大値を先に拾う
	int maxUid = -1;
	for (auto& pair : nodes) {
		auto* v = static_cast<BehaviorNodeView*>(pair.second.get());
		if (!v) continue;
		maxUid = std::max(maxUid, v->GetNodeUID());
	}
	int nextId = maxUid + 1;

	for (auto& pair : nodes) {
		auto* viewNode = static_cast<BehaviorNodeView*>(pair.second.get());
		if (!viewNode) continue;

		int uid = viewNode->GetNodeUID();
		if (uid < 0) {
			uid = nextId++;
			viewNode->SetNodeUID(uid);
		}
		nodePtrToUid[viewNode] = uid;

		BehaviorNodeData nodeData;
		nodeData.name = viewNode->getName();
		nodeData.posX = viewNode->getPos().x;
		nodeData.posY = viewNode->getPos().y;
		nodeData.nodeUID = uid;
		viewNode->SaveParameters(nodeData);

		out.editorNodes.push_back(nodeData);
	}

	// 全リンクの保存
	for (auto& link_weak : flowEditor_->getLinks()) {
		if (auto link = link_weak.lock()) {
			BehaviorLinkData linkData;

			// リンクの左側（出力側）と右側（入力側）のノードとピンのインデックスを特定
			auto* outPin = link->left();
			auto* outNode = outPin->getParent();
			linkData.fromNodeUID = nodePtrToUid[outNode];
			const auto& outPins = outNode->getOuts();
			for (int i = 0; i < (int)outPins.size(); ++i) {
				if (outPins[i].get() == outPin) { linkData.fromPinIndex = i; break; }
			}

			auto* inPin = link->right();
			auto* inNode = inPin->getParent();
			linkData.toNodeUID = nodePtrToUid[inNode];
			const auto& inPins = inNode->getIns();
			for (int i = 0; i < (int)inPins.size(); ++i) {
				if (inPins[i].get() == inPin) { linkData.toPinIndex = i; break; }
			}

			// リンクデータを保存リストに追加
			out.editorLinks.push_back(linkData);
		}
	}

	return out;
}

//===============================================================================
// 右クリックメニューのセットアップ
//===============================================================================
void BehaviorTreeEditor::SetupContextMenu() {
	flowEditor_->rightClickPopUpContent([this](ImFlow::BaseNode* node) {
		//Add Node メニュー
		if (ImGui::MenuItem("Add Action")) { flowEditor_->placeNode<ActionNodeView>("NONE"); }
		if (ImGui::MenuItem("Add SetBlackboardBool")) { flowEditor_->placeNode<SetBlackboardBoolNodeView>(); }
		if (ImGui::MenuItem("Add Condition")) { flowEditor_->placeNode<ConditionNodeView>(); }
		if (ImGui::MenuItem("Add ScoreCondition")) { flowEditor_->placeNode<ScoreConditionNodeView>(); }
		if (ImGui::MenuItem("Add Selector")) { flowEditor_->placeNode<SelectorNodeView>(); }
		if (ImGui::MenuItem("Add Sequence")) { flowEditor_->placeNode<SequenceNodeView>(); }
		if (ImGui::MenuItem("Add PlannerSelector")) { flowEditor_->placeNode<PlannerSelectorNodeView>(); }
		if (ImGui::MenuItem("Add WeightSelector")) { flowEditor_->placeNode<WeightSelectorNodeView>(); }

		// ノード上で右クリックしたときだけ削除を表示
		auto* target = dynamic_cast<BehaviorNodeView*>(node);
		if (target) {
			if (ImGui::MenuItem("Delete Node")) {
				DeleteNodeOnly(target);
			}
			if (ImGui::MenuItem("Delete Subtree")) {
				DeleteSubtreeNode(target);
			}
		}
		});
}

//===============================================================================
// nodeの削除(単体)
//===============================================================================
void BehaviorTreeEditor::DeleteNodeOnly(BehaviorNodeView* target) {
	if (!flowEditor_ || !target) return;

	//targetに繋がる全てのリンクを削除する
	for (auto& link_weak : flowEditor_->getLinks()) {

		// リンクの左側（出力側）と右側（入力側）のノードを取得
		auto link = link_weak.lock();
		if (!link) continue;

		auto* leftNode = static_cast<BehaviorNodeView*>(link->left()->getParent());
		auto* rightNode = static_cast<BehaviorNodeView*>(link->right()->getParent());

		if (leftNode == target || rightNode == target) {
			// リンク参照を切る
			link->right()->deleteLink();
			link->left()->deleteLink();
		}
	}

	// Editor管理マップからも削除する
	for (auto it = nodeViewMap_.begin(); it != nodeViewMap_.end(); ) {
		if (it->second == target) {
			it = nodeViewMap_.erase(it);
		} else {
			++it;
		}

		//ノード削除予約
		target->destroy();
	}
}

//===============================================================================
// nodeの削除(サブツリーごと)
//===============================================================================
void BehaviorTreeEditor::DeleteSubtreeNode(BehaviorNodeView* rootView) {
	if (!flowEditor_ || !rootView) return;

	//DFSでサブツリー全体を探索してノードを収集する
	std::vector<BehaviorNodeView*> stack{ rootView };
	std::vector<BehaviorNodeView*> toDelete;

	while (!stack.empty()) {
		BehaviorNodeView* current = stack.back();
		stack.pop_back();
		toDelete.push_back(current);

		// currentの出力ピンに繋がる子ノードを全てスタックに追加する
		for (auto& outPinSh : current->getOuts()) {
			ImFlow::Pin* targetPin = outPinSh.get();
			for (auto& link_weak : flowEditor_->getLinks()) {
				auto link = link_weak.lock();
				if (!link) continue;

				// リンクの左側（出力側）が current の出力ピンと一致する場合、その右側（入力側）に繋がっているノードが子
				if (link && link->left() == targetPin) {
					auto* childView = static_cast<BehaviorNodeView*>(link->right()->getParent());
					stack.push_back(childView);
				}
			}
		}
	}

	//重複削除
	std::sort(toDelete.begin(), toDelete.end());
	toDelete.erase(std::unique(toDelete.begin(), toDelete.end()), toDelete.end());

	// 収集したノードを削除する
	for (auto* view : toDelete) {
		DeleteNodeOnly(view);
	}

}

//===============================================================================
// 指定したViewノードから再帰的に BehaviorNodeData を構築する
//===============================================================================
BehaviorNodeData BehaviorTreeEditor::BuildRecursiveNodeData(BehaviorNodeView* viewNode) const {
	BehaviorNodeData data;
	if (!viewNode) return data;

	// 基本情報とパラメータの保存
	data.name = viewNode->getName();
	data.nodeUID = viewNode->GetNodeUID(); // 照合に使う UID を保持
	viewNode->SaveParameters(data);

	// 全リンクを取得。Pin::getLinks()がないため、エディタ全体からこのノードの出力ピンに繋がるものを探す
	const auto& allLinks = flowEditor_->getLinks();

	//すぐに再帰呼び出しせず、子ノードのViewを一時的にベクターに収集する
	std::vector<BehaviorNodeView*> childViews;


	// 出力ピンに繋がっている子ノードを再帰的に収集
	// 出力ピンの順番(Child0, Child1...)を維持するため、outPinsのインデックス順に回す
	for (auto& outPinSh : viewNode->getOuts()) {
		ImFlow::Pin* targetPin = outPinSh.get();

		for (auto& link_weak : allLinks) {
			std::shared_ptr<ImFlow::Link> link = link_weak.lock();
			if (link && link->left() == targetPin) {
				// リンクの右側（入力側）に繋がっているノードが子
				ImFlow::BaseNode* pBaseNode = link->right()->getParent();
				if (pBaseNode) {
					childViews.push_back(static_cast<BehaviorNodeView*>(pBaseNode));
				}
			}
		}
	}

	//収集した子ノードをY座標が小さい順にソートする（エディタ上の見た目順）
	std::sort(childViews.begin(), childViews.end(), [](BehaviorNodeView* a, BehaviorNodeView* b) {
		return a->getPos().y < b->getPos().y;
		});

	// ソートされた順番で再帰的に子ノードのデータを構築し、childrenに追加する
	for (BehaviorNodeView* childView : childViews) {
		data.children.push_back(BuildRecursiveNodeData(childView));
	}

	return data;
}
