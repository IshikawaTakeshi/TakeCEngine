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

	//==============================================================================
	// functions
	//==============================================================================

	/// <summary>
	/// コンストラクタとデストラクタ
	/// </summary>
	BehaviorTreeEditor();
	~BehaviorTreeEditor();

	/// <summary>
	/// エディタの初期化。ImNodeFlowのセットアップと右クリックメニューの登録を行う。
	/// </summary>
	void Initialize();

	/// <summary>
	/// ImGuiを用いてエディタを描画する
	/// </summary>
	/// <param name="activeRoot"></param>
	void UpdateImGui(BehaviorNode* activeRoot = nullptr);

	// 反映ボタンが押された時のコールバックを登録
	void SetApplyCallback(std::function<void(const ComboSetData&)> callback) { applyCallback_ = callback; }

	/// <summary>
	/// Enemyからツリーを読み込む
	/// </summary>
	/// <param name="rootNode"></param>
	/// <param name="blackboard"></param>
	void LoadTreeFromEnemy(BehaviorNode* rootNode, Blackboard* blackboard);

	/// <summary>
	/// コンボセットのデータからツリーを構築して読み込む
	/// </summary>
	/// <param name="filepath"></param>
	void LoadTreeFromJson(const std::string& filepath);

	/// <summary>
	/// コンボセットの保存
	/// </summary>
	void SaveComboSet();

	/// <summary>
	/// エディタの状態からComboSetDataを構築する
	/// </summary>
	/// <returns></returns>
	ComboSetData BuildComboSetDataFromEditor() const;

private:

	/// <summary>
	/// 右クリックメニューのセットアップ
	/// </summary>
	void SetupContextMenu();

	// ノード削除（単体 / サブツリー）
	void DeleteNodeOnly(BehaviorNodeView* target);
	void DeleteSubtreeNode(BehaviorNodeView* rootView);

	/// <summary>
	// 指定したViewノードから再帰的に BehaviorNodeData を構築する
	/// </summary>
	BehaviorNodeData BuildRecursiveNodeData(BehaviorNodeView* viewNode) const;

	/// <summary>
	// ロジックからViewを構築する (内部用)
	/// </summary>
	ImFlow::BaseNode* BuildNodeView(BehaviorNode* node, ImVec2& minPos);

	/// <summary>
	/// ロジックノードからノードデータを構築する (内部用)
	/// </summary>
	/// <param name="node"></param>
	/// <returns></returns>
	BehaviorNodeData BuildNodeDataFromLogicNode(const BehaviorNode* node) const;

	/// <summary>
	/// ノードのタイプを文字列で検出する (内部用)
	/// </summary>
	/// <param name="node"></param>
	/// <returns></returns>
	std::string DetectRootType(const BehaviorNode* node) const;

	/// <summary>
	// ノードビューを生成する (内部用)
	/// </summary>
	std::shared_ptr<BehaviorNodeView> CreateNodeView(const std::string& type, const ImVec2& pos, const std::string& name = "NONE");

	/// <summary>
	//ロジックツリーを再帰的に再構築する
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildLogicTree(const BehaviorNodeData& data, std::map<int, std::shared_ptr<BehaviorNodeView>>& idToNode);

	/// <summary>
	// 実行中のロジックツリーとUIを再紐付けする
	/// </summary>
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