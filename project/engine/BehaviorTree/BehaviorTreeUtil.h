#pragma once
#include <string>
#include <utility>
#include <vector>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
// ノードデータ（JSONシリアライズ用 / エディタ・ゲーム共用）
//============================================================================
/// <summary>
/// BehaviorNodeDataに関するデータを保持する構造体です。
/// </summary>
struct BehaviorNodeData {
	std::string name = "UnnamedNode";          // ノード名
	std::string nodeType = "ACTION";           // ノードタイプ（文字列）
	std::string targetState = "NONE";          // ACTION用: ゲーム側で解決するアクションID（旧JSON互換名）

	std::string field = "";                    // 比較する対象（"energy", "hp", "distance" 等）
	std::string op = ">=";                     // 比較演算子（">=", "<=", ">", "<", "==", "!="）
	float conditionThreshold = 0.0f;           // CONDITION用: 閾値
	std::vector<BehaviorNodeData> children;    // 子ノード（SEQUENCE/SELECTOR用・ゲーム実行用）

	// エディタ用拡張
	float posX = 0.0f;                         // X座標
	float posY = 0.0f;                         // Y座標
	int nodeUID = -1;                          // 保存用一意識別子
	float sizeW = 0.0f;                        // ノード幅（0 = 自動）
	float sizeH = 0.0f;                        // ノード高さ（0 = 自動）

	// SetBlackboardBoolNode 用拡張
	std::string bbKey = "";                    // 書き込む対象のキー
	bool bbValue = false;                      // 書き込むbool値
	std::string bbStringValue = "";            // 書き込むstring値（SetBlackboardStringNode 用）

	// WaitNode 用拡張
	float waitTime = 1.0f;                     // 待機時間 [s]（WaitNode 用）

	// ゲーム固有ノードが自由に利用する拡張プロパティ
	nlohmann::json properties = nlohmann::json::object();

};

//============================================================================
// リンクデータ（エディタ接続保存用）
//============================================================================
/// <summary>
/// BehaviorLinkDataに関するデータを保持する構造体です。
/// </summary>
struct BehaviorLinkData {
	int fromNodeUID = -1;
	int fromPinIndex = 0;
	int toNodeUID = -1;
	int toPinIndex = 0;
};

/// <summary>
/// エディタ上の1ノードについて、実行時パラメータと表示レイアウトを分離して保持します。
/// </summary>
struct BehaviorEditorNodeData {
	BehaviorNodeData node;
	float posX = 0.0f;
	float posY = 0.0f;
	float sizeW = 0.0f;
	float sizeH = 0.0f;
};

/// <summary>
/// BehaviorTreeEditorでのみ使用するノード配置と接続情報を保持します。
/// </summary>
struct BehaviorTreeEditorData {
	std::vector<BehaviorEditorNodeData> nodes;
	std::vector<BehaviorLinkData> links;
};

/// <summary>
/// ゲーム固有のコンボ概念に依存しないBehaviorTreeの保存単位です。
/// </summary>
struct BehaviorTreeAsset {
	static constexpr int kCurrentFormatVersion = 1;

	int formatVersion = kCurrentFormatVersion;
	std::string name = "BehaviorTree";
	BehaviorNodeData root;
	BehaviorTreeEditorData editor;
};

//============================================================================
// コンボデータ（1つのコンボセット）
//============================================================================
/// <summary>
/// ComboDataに関するデータを保持する構造体です。
/// </summary>
struct ComboData {
	std::string comboName = "UnnamedCombo";    // コンボ名
	BehaviorNodeData rootNode;                 // ルートノード
};

//============================================================================
// コンボセットデータ（複数のコンボをまとめたもの / エディタ保存・読み込み単位）
//============================================================================
/// <summary>
/// ComboSetDataに関するデータを保持する構造体です。
/// </summary>
struct ComboSetData {
	std::string setName = "DefaultComboSet";   // セット名
	std::vector<ComboData> combos;             // コンボリスト（ゲーム実行用ツリー）
	std::string rootType = "SELECTOR";         // ルートのコンポジットタイプ

	// エディタ用フラットレイアウト情報（孤立ノードを含む全ノードの状態）
	std::vector<BehaviorNodeData> editorNodes; // 全ノードのリスト
	std::vector<BehaviorLinkData> editorLinks; // 全接続情報のリスト
};




void to_json(nlohmann::json& j, const ComboSetData& data);
void from_json(const nlohmann::json& j, ComboSetData& data);

void to_json(nlohmann::json& j, const BehaviorTreeAsset& data);
void from_json(const nlohmann::json& j, BehaviorTreeAsset& data);

void to_json(nlohmann::json& j, const BehaviorEditorNodeData& data);
void from_json(const nlohmann::json& j, BehaviorEditorNodeData& data);

void to_json(nlohmann::json& j, const BehaviorNodeData& data);
void from_json(const nlohmann::json& j, BehaviorNodeData& data);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BehaviorLinkData, fromNodeUID, fromPinIndex, toNodeUID, toPinIndex)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BehaviorTreeEditorData, nodes, links)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComboData, comboName, rootNode)

/// <summary>旧ComboSet形式を汎用BehaviorTreeAssetへ変換します。</summary>
BehaviorTreeAsset ConvertComboSetToBehaviorTreeAsset(const ComboSetData& legacyData);

/// <summary>互換API向けにBehaviorTreeAssetを旧ComboSet形式へ変換します。</summary>
ComboSetData ConvertBehaviorTreeAssetToComboSet(const BehaviorTreeAsset& asset);
//JSONディレクトリパスの定義
TAKEC_DEFINE_JSON_DIRECTORY_PATH(BehaviorTreeAsset, "JsonLoader/BehaviorTree");
TAKEC_DEFINE_JSON_DIRECTORY_PATH(ComboSetData, "JsonLoader/ComboSet");
