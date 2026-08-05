#pragma once
#include <map>
#include <functional>
#include <filesystem>
#include <optional>
#include <string>
#include <memory>
#include <utility>
#include <unordered_set>
#include <vector>

// Engine
#include "BehaviorTreeUtil.h"
#include "BehaviorTreeAssetValidator.h"
#include "BehaviorNodeRegistry.h"
#include "engine/BehaviorTree/View/BlackboardPanel.h"
#include "View/BehaviorNodeViewRegistry.h"

// ImNodeFlow
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include "View/BehaviorNodeView.h"

namespace TakeC { class JsonLoader; }

// 前方宣言
class BehaviorNode;

//==================================================================================
// BehaviorTreeEditor class
//==================================================================================
/// <summary>
/// BehaviorTreeEditorに関するデータを表示・編集する開発支援クラスです。
/// </summary>
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
	/// ゲーム側で構成したRuntimeノードレジストリを使用します。
	/// </summary>
	void SetNodeRegistry(TakeC::BehaviorNodeRegistry& registry) noexcept { nodeRegistry_ = &registry; }

	TakeC::BehaviorNodeRegistry& NodeRegistry() noexcept { return *nodeRegistry_; }
	TakeC::BehaviorNodeViewRegistry& ViewRegistry() noexcept { return viewRegistry_; }

	/// <summary>Editorが利用するJsonLoaderをゲーム側から注入します。</summary>
	void SetJsonLoader(TakeC::JsonLoader& jsonLoader);

	/// <summary>BehaviorTree JSONの保存・読込先をゲーム側から指定します。</summary>
	void SetAssetDirectory(std::filesystem::path directory);

	/// <summary>保存・読込先を従来のJsonPath既定値へ戻します。</summary>
	void ClearAssetDirectory();

	/// <summary>
	/// ImGuiを用いてエディタを描画する
	/// </summary>
	/// <param name="activeRoot"></param>
	void UpdateImGui(BehaviorNode* activeRoot = nullptr);

	/// <summary>反映ボタンで汎用BehaviorTreeAssetを受け取るコールバックを登録します。</summary>
	void SetApplyAssetCallback(std::function<void(const BehaviorTreeAsset&)> callback) { applyAssetCallback_ = std::move(callback); }

	/// <summary>旧ComboSetDataを利用するゲームコード向けの互換コールバックです。</summary>
	void SetApplyCallback(std::function<void(const ComboSetData&)> callback) { legacyApplyCallback_ = std::move(callback); }

	/// <summary>
	/// 実行中のツリーとBlackboardをEditorへ読み込む
	/// </summary>
	/// <param name="rootNode"></param>
	/// <param name="blackboard"></param>
	void LoadTree(BehaviorNode* rootNode, Blackboard* blackboard);

	// 旧ゲームAPIとの移行用ラッパー
	void LoadTreeFromEnemy(BehaviorNode* rootNode, Blackboard* blackboard) {
		LoadTree(rootNode, blackboard);
	}

	/// <summary>
	/// コンボセットのデータからツリーを構築して読み込む
	/// </summary>
	/// <param name="filepath"></param>
	void LoadTreeFromJson(const std::string& filepath);

	/// <summary>
	/// コンボセットの保存
	/// </summary>
	void SaveComboSet();

	/// <summary>現在のツリーを汎用BehaviorTreeAssetとして保存します。</summary>
	void SaveBehaviorTree();

	/// <summary>エディタの状態からBehaviorTreeAssetを構築します。</summary>
	BehaviorTreeAsset BuildBehaviorTreeAssetFromEditor() const;

	/// <summary>直近の読込・保存・Apply時に得られた検証結果を返します。</summary>
	const TakeC::BehaviorTreeValidationResult& ValidationResult() const noexcept { return validationResult_; }

	/// <summary>
	/// エディタの状態からComboSetDataを構築する
	/// </summary>
	/// <returns></returns>
	ComboSetData BuildComboSetDataFromEditor() const;

	/// <summary>
	// 実行中のロジックツリーとUIを再紐付けする
	/// </summary>
	void SyncWithActiveTree(BehaviorNode* root, Blackboard* blackboard = nullptr);

private:

	/// <summary>
	/// 右クリックメニューのセットアップ
	/// </summary>
	void SetupContextMenu();
	void RegisterBuiltInNodeViews();
	void RefreshAssetNames();
	void DrawValidationIssues() const;
	TakeC::JsonLoader* ResolveJsonLoader() const;

	// ノード削除（単体 / サブツリー）
	void DeleteNodeOnly(BehaviorNodeView* target);
	void DeleteSubtreeNode(BehaviorNodeView* rootView);

	/**
	 * @brief Blackboardのキー名リストを再収集し、全 ConditionNodeView に注入する
	 */
	void UpdateBlackboardKeys();

	/// <summary>
	// 指定したViewノードから再帰的に BehaviorNodeData を構築する
	/// </summary>
	BehaviorNodeData BuildRecursiveNodeData(BehaviorNodeView* viewNode) const;
	BehaviorNodeData BuildRecursiveNodeDataInternal(
		BehaviorNodeView* viewNode,
		std::unordered_set<BehaviorNodeView*>& visiting) const;

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
	std::shared_ptr<BehaviorNodeView> CreateNodeView(const BehaviorNodeData& data, const ImVec2& pos);

	/// <summary>
	//ロジックツリーを再帰的に再構築する
	/// </summary>
	std::unique_ptr<BehaviorNode> BuildLogicTree(const BehaviorNodeData& data, std::map<int, std::shared_ptr<BehaviorNodeView>>& idToNode);

private:

	BehaviorNode* rootNode_ = nullptr;
	Blackboard* blackboard_ = nullptr;
	std::unique_ptr<ImFlow::ImNodeFlow> flowEditor_;
	std::map<BehaviorNode*, ImFlow::BaseNode*> nodeViewMap_;
	std::vector<std::string> assetNames_;
	BehaviorTreeAsset currentAsset_;
	std::optional<std::filesystem::path> assetDirectory_;
	TakeC::JsonLoader* jsonLoader_ = nullptr;
	TakeC::BehaviorTreeValidationResult validationResult_;
	bool hasLoadError_ = false;

	// 反映実行用のコールバック
	std::function<void(const BehaviorTreeAsset&)> applyAssetCallback_;
	std::function<void(const ComboSetData&)> legacyApplyCallback_;

	// JSONロード時に構築したツリーの管理
	std::unique_ptr<BehaviorNode> ownedRootNode_;

	// Blackboard から毎フレーム収集したキー名リスト（ConditionNodeView の Field Combo 用）
	std::vector<std::string> blackboardKeys_;
	TakeC::BlackboardPanel blackboardPanel_;
	TakeC::BehaviorNodeRegistry defaultNodeRegistry_;
	TakeC::BehaviorNodeRegistry* nodeRegistry_ = nullptr;
	TakeC::BehaviorNodeViewRegistry viewRegistry_;
};
