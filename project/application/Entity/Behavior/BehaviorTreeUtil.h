#pragma once
#include <vector>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"
#include "application/Entity/State/GameCharacterState.h"

//============================================================================
// ノードの種類
//============================================================================
enum class BehaviorNodeType {
	ACTION,           // 行動ノード（ステート遷移）
	CONDITION,        // 条件ノード
	SEQUENCE,         // シーケンスノード
	SELECTOR,         // セレクターノード
	PLANNER_SELECTOR, // プランナーセレクター
	WEIGHT_SELECTOR,  // ウェイトセレクター
	SCORE_CONDITION,  // スコア条件
	SET_BB_BOOL,      // Blackboard に bool をセット
	SET_BB_STRING,    // Blackboard に string をセット
};

//============================================================================
// ノードデータ（JSONシリアライズ用 / エディタ・ゲーム共用）
//============================================================================
struct BehaviorNodeData {
	std::string name = "UnnamedNode";          // ノード名
	std::string nodeType = "ACTION";           // ノードタイプ（文字列）
	std::string targetState = "NONE";          // ACTION用: 遷移先ステート（文字列）

	std::string field = "";                    // 比較する対象（"energy", "hp", "distance" 等）
	std::string op = ">=";                     // 比較演算子（">=", "<=", ">", "<", "==", "!="）
	float conditionThreshold = 0.0f;           // CONDITION用: 閾値
	std::vector<BehaviorNodeData> children;    // 子ノード（SEQUENCE/SELECTOR用・ゲーム実行用）

	// エディタ用拡張
	float posX = 0.0f;                         // X座標
	float posY = 0.0f;                         // Y座標
	int nodeUID = -1;                          // 保存用一意識別子

	// SetBlackboardBoolNode 用拡張
	std::string bbKey = "";                    // 書き込む対象のキー
	bool bbValue = false;                      // 書き込むbool値
	std::string bbStringValue = "";            // 書き込むstring値（SetBlackboardStringNode 用）

	
};

//============================================================================
// リンクデータ（エディタ接続保存用）
//============================================================================
struct BehaviorLinkData {
	int fromNodeUID = -1;
	int fromPinIndex = 0;
	int toNodeUID = -1;
	int toPinIndex = 0;
};

//============================================================================
// コンボデータ（1つのコンボセット）
//============================================================================
struct ComboData {
	std::string comboName = "UnnamedCombo";    // コンボ名
	BehaviorNodeData rootNode;                 // ルートノード
};

//============================================================================
// コンボセットデータ（複数のコンボをまとめたもの / エディタ保存・読み込み単位）
//============================================================================
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

void to_json(nlohmann::json& j, const BehaviorNodeData& data);
void from_json(const nlohmann::json& j, BehaviorNodeData& data);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BehaviorLinkData, fromNodeUID, fromPinIndex, toNodeUID, toPinIndex)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComboData, comboName, rootNode)
//JSONディレクトリパスの定義
TAKEC_DEFINE_JSON_DIRECTORY_PATH(ComboSetData, "Resources/JsonLoader/ComboSet/");
