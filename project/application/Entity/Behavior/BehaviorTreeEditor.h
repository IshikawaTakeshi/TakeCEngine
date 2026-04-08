#pragma once
#include <map>
#include <functional>
#include <string>
#include <memory>
#include <vector>

// Engine
#include "application/Entity/Behavior/BehaviorTreeUtil.h"

// ImNodeFlow
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include "application/Entity/Behavior/View/BehaviorNodeView.h"

// 前方宣言
class BehaviorNode;
class Blackboard;

//==================================================================================
// BehaviorTreeEditor class
//==================================================================================
class BehaviorTreeEditor {
public:
	BehaviorTreeEditor();
	~BehaviorTreeEditor();

	void Initialize();
	void UpdateImGui(BehaviorNode* activeRoot = nullptr);

	// 反映ボタンが押された時のコールバックを登録
	void SetApplyCallback(std::function<void(const ComboSetData&)> callback) { applyCallback_ = callback; }

	void LoadTreeFromEnemy(BehaviorNode* rootNode, Blackboard* blackboard);
	void LoadTreeFromJson(const std::string& filepath);
	void SaveComboSet();

	ComboSetData BuildComboSetDataFromEditor() const;

private:
	// 指定したViewノードから再帰的に BehaviorNodeData を構築する
	BehaviorNodeData BuildRecursiveNodeData(BehaviorNodeView* viewNode) const;

	// ロジックからViewを構築する (内部用)
	ImFlow::BaseNode* BuildNodeView(BehaviorNode* node, ImVec2& minPos);

	BehaviorNodeData BuildNodeDataFromLogicNode(const BehaviorNode* node) const;
	std::string DetectRootType(const BehaviorNode* node) const;

	// ノードビューを生成する (内部用)
	std::shared_ptr<BehaviorNodeView> CreateNodeView(const std::string& type, const ImVec2& pos, const std::string& name = "NONE");
	//ロジックツリーを再帰的に再構築する
	std::unique_ptr<BehaviorNode> BuildLogicTree(const BehaviorNodeData& data, std::map<int, std::shared_ptr<BehaviorNodeView>>& idToNode);
	// 実行中のロジックツリーとUIを再紐付けする
	void SyncWithActiveTree(BehaviorNode* root);

private:
	BehaviorNode* rootNode_ = nullptr;
	Blackboard* blackboard_ = nullptr;
	std::unique_ptr<ImFlow::ImNodeFlow> flowEditor_;
	std::map<BehaviorNode*, ImFlow::BaseNode*> nodeViewMap_;
	std::vector<std::string> comboSetNames_;
	ComboSetData currentComboSetData_;

	// 反映実行用のコールバック
	std::function<void(const ComboSetData&)> applyCallback_;

	// JSONロード時に構築したツリーの管理
	std::unique_ptr<BehaviorNode> ownedRootNode_;
};