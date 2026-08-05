#include "BehaviorTreeEditor.h"
// Logic
#include "engine/BehaviorTree/BehaviorNode.h"
#include "engine/BehaviorTree/Blackboard.h"
#include "engine/BehaviorTree/CompositeNode.h"
#include "engine/BehaviorTree/SelectorNode.h"
#include "engine/BehaviorTree/SequenceNode.h"
#include "engine/BehaviorTree/ConditionNode.h"
#include "engine/BehaviorTree/ScoreConditionNode.h"
#include "engine/BehaviorTree/PlannerSelectorNode.h"
#include "engine/BehaviorTree/ActionNode.h"
#include "engine/BehaviorTree/WeightSelectorNode.h"
#include "engine/BehaviorTree/SetBlackboardBoolNode.h"
#include "engine/BehaviorTree/SetBlackboardStringNode.h"
#include "engine/BehaviorTree/WaitNode.h"
#include "engine/BehaviorTree/WaitBlackboardTimeNode.h"

// Views
#include "engine/BehaviorTree/View/ActionNodeView.h"
#include "engine/BehaviorTree/View/SelectorNodeView.h"
#include "engine/BehaviorTree/View/ConditionNodeView.h"
#include "engine/BehaviorTree/View/SequenceNodeView.h"
#include "engine/BehaviorTree/View/ScoreConditionNodeView.h"
#include "engine/BehaviorTree/View/PlannerSelectorNodeView.h"
#include "engine/BehaviorTree/View/WeightSelectorNodeView.h"
#include "engine/BehaviorTree/View/SetBlackboardBoolNodeView.h"
#include "engine/BehaviorTree/View/SetBlackboardStringNodeView.h"
#include "engine/BehaviorTree/View/WaitNodeView.h"
#include "engine/BehaviorTree/View/WaitBlackboardTimeNodeView.h"

#include "engine/Base/ImGuiManager.h"
#include "engine/Base/TakeCFrameWork.h"

#include <algorithm>

using namespace TakeC;

BehaviorTreeEditor::BehaviorTreeEditor()
	: nodeRegistry_(&defaultNodeRegistry_) {
	TakeC::RegisterBuiltInBehaviorNodes(defaultNodeRegistry_);
	RegisterBuiltInNodeViews();
}
BehaviorTreeEditor::~BehaviorTreeEditor() = default;

void BehaviorTreeEditor::RegisterBuiltInNodeViews() {
	auto registerView = [this](TakeC::BehaviorNodeViewRegistration registration) {
		viewRegistry_.Register(std::move(registration), true);
	};

	registerView({"ACTION", "Action", "Leaf",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<ActionNodeView>(pos, data.targetState);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<ActionNodeView>("NONE"); }});
	registerView({"CONDITION", "Condition", "Leaf",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<ConditionNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<ConditionNodeView>(); }});
	registerView({"SCORE_CONDITION", "Score Condition", "Leaf",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<ScoreConditionNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<ScoreConditionNodeView>(); }});
	registerView({"WAIT", "Wait", "Leaf",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<WaitNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<WaitNodeView>(); }});
	registerView({"SET_BB_BOOL", "Set Blackboard Bool", "Blackboard",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<SetBlackboardBoolNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<SetBlackboardBoolNodeView>(); }});
	registerView({"SET_BB_STRING", "Set Blackboard String", "Blackboard",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<SetBlackboardStringNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<SetBlackboardStringNodeView>(); }});
	registerView({"WAIT_BB_TIME", "Wait Blackboard Time", "Blackboard",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<WaitBlackboardTimeNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<WaitBlackboardTimeNodeView>(); }});
	registerView({"SELECTOR", "Selector", "Composite",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<SelectorNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<SelectorNodeView>(); }});
	registerView({"SEQUENCE", "Sequence", "Composite",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<SequenceNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<SequenceNodeView>(); }});
	registerView({"PLANNER_SELECTOR", "Planner Selector", "Composite",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<PlannerSelectorNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<PlannerSelectorNodeView>(); }});
	registerView({"WEIGHT_SELECTOR", "Weight Selector", "Composite",
		[](ImFlow::ImNodeFlow& editor, const ImVec2& pos, const BehaviorNodeData& data) {
			auto view = editor.addNode<WeightSelectorNodeView>(pos);
			view->LoadParameters(data);
			return view;
		},
		[](ImFlow::ImNodeFlow& editor) { editor.placeNode<WeightSelectorNodeView>(); }});
}

//==================================================================================
// 初期化
//==================================================================================
void BehaviorTreeEditor::Initialize() {

	// ImNodeFlowエディタの初期化
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");

	//右クリックメニューのノードを追加可能にする
	SetupContextMenu();

	RefreshAssetNames();
	
}

void BehaviorTreeEditor::SetAssetDirectory(std::filesystem::path directory) {
	if (directory.empty()) {
		ClearAssetDirectory();
		return;
	}
	assetDirectory_ = std::move(directory).lexically_normal();
	if (flowEditor_) RefreshAssetNames();
}

void BehaviorTreeEditor::ClearAssetDirectory() {
	assetDirectory_.reset();
	if (flowEditor_) RefreshAssetNames();
}

void BehaviorTreeEditor::SetJsonLoader(TakeC::JsonLoader& jsonLoader) {
	jsonLoader_ = &jsonLoader;
	if (flowEditor_) RefreshAssetNames();
}

TakeC::JsonLoader* BehaviorTreeEditor::ResolveJsonLoader() const {
	return jsonLoader_ ? jsonLoader_ : TakeC::TakeCFrameWork::GetJsonLoader();
}

void BehaviorTreeEditor::RefreshAssetNames() {
	auto* jsonLoader = ResolveJsonLoader();
	if (!jsonLoader) return;

	if (assetDirectory_) {
		// 新形式のfrom_jsonが旧ComboSet形式も変換できるため、同一フォルダーを一度だけ列挙する。
		assetNames_ = jsonLoader->GetJsonDataListAt<BehaviorTreeAsset>(*assetDirectory_);
	} else {
		assetNames_ = jsonLoader->GetJsonDataList<BehaviorTreeAsset>();
		const auto legacyNames = jsonLoader->GetJsonDataList<ComboSetData>();
		assetNames_.insert(assetNames_.end(), legacyNames.begin(), legacyNames.end());
	}

	std::sort(assetNames_.begin(), assetNames_.end());
	assetNames_.erase(std::unique(assetNames_.begin(), assetNames_.end()), assetNames_.end());
}

//==================================================================================
// ロジックツリーを再帰的に再構築する
//==================================================================================
std::unique_ptr<BehaviorNode> BehaviorTreeEditor::BuildLogicTree(const BehaviorNodeData& data, std::map<int, std::shared_ptr<BehaviorNodeView>>& idToNode) {
	std::unique_ptr<BehaviorNode> node = nodeRegistry_->Create(data);

	if (!node) return nullptr;

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
void BehaviorTreeEditor::SyncWithActiveTree(BehaviorNode* root, Blackboard* blackboard) {
	if (blackboard) {
		blackboard_ = blackboard;
	}
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

	// Blackboard のキーリストを更新して全 ConditionNodeView に注入
	UpdateBlackboardKeys();

	if (flowEditor_) {
		ImGui::Begin("Behavior Tree Editor");

		// Blackboardの表示
		if (ImGui::TreeNode("Blackboard")) {
			if (blackboard_) {
				blackboardPanel_.Draw(*blackboard_);
			}
			ImGui::TreePop();
		}
		ImGui::Separator();
		SaveBehaviorTree();
		DrawValidationIssues();

		// エネミーへの反映ボタン

		if (ImGui::Button("Apply Behavior Tree")) {
			currentAsset_ = BuildBehaviorTreeAssetFromEditor();
			validationResult_ = TakeC::BehaviorTreeAssetValidator::Validate(currentAsset_, *nodeRegistry_);
			hasLoadError_ = false;
			if (validationResult_.IsValid() && applyAssetCallback_) {
				applyAssetCallback_(currentAsset_);
			}
			if (validationResult_.IsValid() && legacyApplyCallback_) {
				legacyApplyCallback_(ConvertBehaviorTreeAssetToComboSet(currentAsset_));
			}
		}

		if (ImGui::CollapsingHeader("Load Behavior Tree")) {
			int currentPresetIndex_ = -1;
			bool isSelected = false;
			isSelected = ImGuiManager::ComboBoxString("Behavior Tree", assetNames_, currentPresetIndex_);

			if (isSelected) {
				LoadTreeFromJson(assetNames_[currentPresetIndex_] + ".json");
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
// 実行中のツリーを読み込む
//===============================================================================
void BehaviorTreeEditor::LoadTree(BehaviorNode* rootNode, Blackboard* blackboard) {
	if (!flowEditor_ || !rootNode) return;

	blackboard_ = blackboard;
	rootNode_ = rootNode;
	// いったんエディタのノードをクリアする
	nodeViewMap_.clear();
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");
	// 右クリックメニューを再設定
	SetupContextMenu();

	currentAsset_.name = rootNode->GetName();
	currentAsset_.root = BuildNodeDataFromLogicNode(rootNode);
	ImVec2 startPos = ImVec2(100.0f, 100.0f);
	BuildNodeView(rootNode_, startPos);
	currentAsset_ = BuildBehaviorTreeAssetFromEditor();
	hasLoadError_ = false;
}

//===============================================================================
// コンボセットのデータからツリーを構築して読み込む
//===============================================================================
void BehaviorTreeEditor::LoadTreeFromJson(const std::string& filepath) {
	if (!flowEditor_) return;

	auto* jsonLoader = ResolveJsonLoader();
	BehaviorTreeAsset loadedAsset;
	if (assetDirectory_ &&
		jsonLoader->IsJsonDataExistsAt<BehaviorTreeAsset>(*assetDirectory_, filepath)) {
		loadedAsset = jsonLoader->LoadJsonDataAt<BehaviorTreeAsset>(*assetDirectory_, filepath);
	} else if (assetDirectory_) {
		return;
	} else if (jsonLoader->IsJsonDataExists<BehaviorTreeAsset>(filepath)) {
		loadedAsset = jsonLoader->LoadJsonData<BehaviorTreeAsset>(filepath);
	} else if (jsonLoader->IsJsonDataExists<ComboSetData>(filepath)) {
		loadedAsset = ConvertComboSetToBehaviorTreeAsset(
			jsonLoader->LoadJsonData<ComboSetData>(filepath));
	} else {
		return;
	}
	validationResult_ = TakeC::BehaviorTreeAssetValidator::Validate(loadedAsset, *nodeRegistry_);
	if (!validationResult_.IsValid()) {
		hasLoadError_ = true;
		return;
	}
	currentAsset_ = std::move(loadedAsset);
	hasLoadError_ = false;

	// FIX: flowEditor_を再作成する前にマップをクリア
	nodeViewMap_.clear();
	rootNode_ = nullptr;  // 古い参照も無効化

	// エディタをクリア
	flowEditor_ = std::make_unique<ImFlow::ImNodeFlow>("BehaviorTreeEditor");
	// 右クリックメニューを再設定
	SetupContextMenu();

	std::map<int, std::shared_ptr<BehaviorNodeView>> idToNode;

	// 1. ノードの生成
	for (const auto& editorNode : currentAsset_.editor.nodes) {
		const auto& nodeData = editorNode.node;
		ImVec2 pos = ImVec2(editorNode.posX, editorNode.posY);
		auto v = CreateNodeView(nodeData, pos);

		if (v) {
			v->SetNodeUID(nodeData.nodeUID); // 同期に使う JSON UID をセット
			v->SetUserSize(ImVec2(editorNode.sizeW, editorNode.sizeH)); // サイズ情報を復元
			idToNode[nodeData.nodeUID] = v;
		}
	}

	// 2. リンクの構築
	for (const BehaviorLinkData& linkData : currentAsset_.editor.links) {
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
	std::unique_ptr<BehaviorNode> rootLogicNode = BuildLogicTree(currentAsset_.root, idToNode);
	if (!rootLogicNode) return;

	// 4. エディタの状態を更新
	ownedRootNode_ = std::move(rootLogicNode);
	rootNode_ = ownedRootNode_.get();

	// Editor情報を省略したランタイム用Assetでも編集を開始できるようにする。
	if (currentAsset_.editor.nodes.empty()) {
		ImVec2 startPos = ImVec2(100.0f, 100.0f);
		BuildNodeView(rootNode_, startPos);
		currentAsset_ = BuildBehaviorTreeAssetFromEditor();
	}
}

//===============================================================================
// ツリーををコンボセットとしてファイルに保存する
//===============================================================================
void BehaviorTreeEditor::SaveComboSet() {
	SaveBehaviorTree();
}

void BehaviorTreeEditor::SaveBehaviorTree() {
	if (!flowEditor_) return;
	if (hasLoadError_) return;

	const std::string previousName = currentAsset_.name;
	currentAsset_ = BuildBehaviorTreeAssetFromEditor();
	if (!previousName.empty()) currentAsset_.name = previousName;
	validationResult_ = TakeC::BehaviorTreeAssetValidator::Validate(currentAsset_, *nodeRegistry_);
	if (!validationResult_.IsValid()) return;

	const std::filesystem::path directory = assetDirectory_.value_or(std::filesystem::path{});
	const bool saved = ImGuiManager::ShowSavePopup(
		ResolveJsonLoader(),
		"Save Behavior Tree",
		std::string(currentAsset_.name + ".json").c_str(),
		currentAsset_,
		currentAsset_.name,
		directory);
	if (saved) RefreshAssetNames();

}

void BehaviorTreeEditor::DrawValidationIssues() const {
	if (validationResult_.Issues().empty()) return;
	const ImGuiTreeNodeFlags flags = validationResult_.IsValid() ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
	if (!ImGui::CollapsingHeader("Validation", flags)) return;

	for (const auto& issue : validationResult_.Issues()) {
		const bool isError = issue.severity == TakeC::BehaviorTreeIssueSeverity::Error;
		const ImVec4 color = isError
			? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
			: ImVec4(1.0f, 0.75f, 0.25f, 1.0f);
		ImGui::TextColored(color, "[%s] %s", issue.code.c_str(), issue.message.c_str());
		if (!issue.nodePath.empty()) {
			ImGui::TextDisabled("  %s (UID: %d)", issue.nodePath.c_str(), issue.nodeUID);
		}
	}
}

//===============================================================================
// 再帰的にノードビューを構築する
//===============================================================================
ImFlow::BaseNode* BehaviorTreeEditor::BuildNodeView(BehaviorNode* node, ImVec2& minPos) {
	if (!node || !flowEditor_) return nullptr;

	ImFlow::BaseNode* viewNode = nullptr;

	// 1. レジストリに登録された型IDからViewを生成
	BehaviorNodeData nodeData = BuildNodeDataFromLogicNode(node);
	auto v = CreateNodeView(nodeData, minPos);

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
std::shared_ptr<BehaviorNodeView> BehaviorTreeEditor::CreateNodeView(
	const BehaviorNodeData& data,
	const ImVec2& pos) {
	if (!flowEditor_) {
		return nullptr;
	}

	std::shared_ptr<BehaviorNodeView> view =
		viewRegistry_.CreateAt(data.nodeType, *flowEditor_, pos, data);
	if (view && !data.name.empty()) {
		view->SetCustomName(data.name);
	}
	return view;
}


//===============================================================================
// ロジックノードから保存用のデータ構造を構築する
//===============================================================================
BehaviorNodeData BehaviorTreeEditor::BuildNodeDataFromLogicNode(const BehaviorNode* node) const {
	BehaviorNodeData data;
	if (!node) return data;

	if (!nodeRegistry_->SerializeParameters(*node, data)) {
		data.nodeType = DetectRootType(node);
		data.name = node->GetName();
		data.nodeUID = node->GetUID();
	}

	if (const auto* composite = dynamic_cast<const CompositeNode*>(node)) {
		for (const auto& child : composite->GetChildren()) {
			data.children.push_back(BuildNodeDataFromLogicNode(child.get()));
		}
	}

	return data;
}

//===============================================================================
// ノードの種類を文字列で判別する（保存用）
//===============================================================================
std::string BehaviorTreeEditor::DetectRootType(const BehaviorNode* node) const {
	if (!node) {
		return "UNKNOWN";
	}
	if (!node->GetTypeId().empty()) {
		return std::string(node->GetTypeId());
	}

	if (dynamic_cast<const ActionNode*>(node)) {
		return "ACTION";
	} else if (dynamic_cast<const SetBlackboardBoolNode*>(node)) {
		return "SET_BB_BOOL";
	} else if (dynamic_cast<const SetBlackboardStringNode*>(node)) {
		return "SET_BB_STRING";
	} else if (dynamic_cast<const WaitBlackboardTimeNode*>(node)) {
		return "WAIT_BB_TIME";
	} else if (dynamic_cast<const WaitNode*>(node)) {
		return "WAIT";
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
	} else if (dynamic_cast<const WeightSelectorNode*>(node)) {
		return "WEIGHT_SELECTOR";
	} else {
		return "UNKNOWN";
	}
}

//===============================================================================
// エディタ上のノード接続（ImNodeFlow）から BehaviorTreeAsset を構成する
//===============================================================================
BehaviorTreeAsset BehaviorTreeEditor::BuildBehaviorTreeAssetFromEditor() const {
	BehaviorTreeAsset out;
	out.name = currentAsset_.name.empty() ? "BehaviorTree" : currentAsset_.name;
	if (!flowEditor_) return out;

	const auto& allLinks = flowEditor_->getLinks();

	auto& nodes = flowEditor_->getNodes();
	// 再帰データとフラットデータが同じUIDを持つよう、先に未採番ノードへUIDを割り当てる。
	int maxUid = -1;
	for (auto& pair : nodes) {
		auto* view = static_cast<BehaviorNodeView*>(pair.second.get());
		if (view) maxUid = std::max(maxUid, view->GetNodeUID());
	}
	int nextId = maxUid + 1;
	for (auto& pair : nodes) {
		auto* view = static_cast<BehaviorNodeView*>(pair.second.get());
		if (view && view->GetNodeUID() < 0) view->SetNodeUID(nextId++);
	}

	// --- 1. ルートノード（入力接続がないノード）を特定し、再帰的に構造を構築 ---
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
			auto rootData = BuildRecursiveNodeData(viewNode);
			rootData.posY = viewNode->getPos().y; // ルート候補の表示順にだけ使用
			out.root.children.push_back(std::move(rootData));
		}
	}

	std::sort(out.root.children.begin(), out.root.children.end(), [](const BehaviorNodeData& a, const BehaviorNodeData& b) {
		return a.posY < b.posY;
		});
	if (out.root.children.size() == 1) {
		out.root = std::move(out.root.children.front());
	} else {
		out.root.name = out.name;
		const std::string& previousType = currentAsset_.root.nodeType;
		const bool isCompositeType = previousType == "SELECTOR" || previousType == "SEQUENCE" ||
			previousType == "PLANNER_SELECTOR" || previousType == "WEIGHT_SELECTOR";
		out.root.nodeType = isCompositeType ? previousType : "SELECTOR";
	}

	// --- 2. エディタレイアウト情報（フラットなリスト）を保存 ---
	std::map<ImFlow::BaseNode*, int> nodePtrToUid;

	for (auto& pair : nodes) {
		auto* viewNode = static_cast<BehaviorNodeView*>(pair.second.get());
		if (!viewNode) continue;

		int uid = viewNode->GetNodeUID();
		nodePtrToUid[viewNode] = uid;

		BehaviorEditorNodeData editorNode;
		editorNode.node.name = viewNode->getName();
		editorNode.node.nodeUID = uid;
		editorNode.posX = viewNode->getPos().x;
		editorNode.posY = viewNode->getPos().y;
		editorNode.sizeW = viewNode->GetUserSize().x;
		editorNode.sizeH = viewNode->GetUserSize().y;
		viewNode->SaveParameters(editorNode.node);

		out.editor.nodes.push_back(std::move(editorNode));
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
			out.editor.links.push_back(linkData);
		}
	}

	return out;
}

ComboSetData BehaviorTreeEditor::BuildComboSetDataFromEditor() const {
	return ConvertBehaviorTreeAssetToComboSet(BuildBehaviorTreeAssetFromEditor());
}

//===============================================================================
// Blackboard のキー名リストを再収集して全 ConditionNodeView に注入する
//===============================================================================
void BehaviorTreeEditor::UpdateBlackboardKeys() {
	if (!flowEditor_) return;

	// Blackboard がなければリストをクリアして終了（フォールバック側に任せる）
	if (!blackboard_) {
		blackboardKeys_.clear();
	} else {
		// キー名を収集（実行のたびに作り直す。量が少ないので許容範囲）
		blackboardKeys_.clear();
		for (const auto& pair : blackboard_->Entries()) {
			blackboardKeys_.push_back(pair.first);
		}
		// 毎回順番が変わらないようにソートしておく
		std::sort(blackboardKeys_.begin(), blackboardKeys_.end());
	}

	// 全ノードを走査して ConditionNodeView / WaitBlackboardTimeNodeView にキーリストを渡す
	for (auto& pair : flowEditor_->getNodes()) {
		if (auto* condView = dynamic_cast<ConditionNodeView*>(pair.second.get())) {
			condView->SetBlackboardKeys(blackboardKeys_);
		}
		if (auto* waitBBView = dynamic_cast<WaitBlackboardTimeNodeView*>(pair.second.get())) {
			waitBBView->SetBlackboardKeys(blackboardKeys_);
		}
	}
}

//===============================================================================
// 右クリックメニューのセットアップ
//===============================================================================
void BehaviorTreeEditor::SetupContextMenu() {
	flowEditor_->rightClickPopUpContent([this](ImFlow::BaseNode* node) {
		std::string currentCategory;
		for (const auto* registration : viewRegistry_.Registrations()) {
			if (registration->category != currentCategory) {
				if (!currentCategory.empty()) {
					ImGui::Separator();
				}
				currentCategory = registration->category;
				ImGui::TextDisabled("%s", currentCategory.c_str());
			}

			if (ImGui::MenuItem(registration->displayName.c_str())) {
				viewRegistry_.Place(registration->typeId, *flowEditor_);
			}
		}

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
	std::unordered_set<BehaviorNodeView*> visiting;
	return BuildRecursiveNodeDataInternal(viewNode, visiting);
}

BehaviorNodeData BehaviorTreeEditor::BuildRecursiveNodeDataInternal(
	BehaviorNodeView* viewNode,
	std::unordered_set<BehaviorNodeView*>& visiting) const {
	BehaviorNodeData data;
	if (!viewNode) return data;
	if (!visiting.insert(viewNode).second) return data;

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
		if (!visiting.contains(childView)) {
			data.children.push_back(BuildRecursiveNodeDataInternal(childView, visiting));
		}
	}
	visiting.erase(viewNode);

	return data;
}
