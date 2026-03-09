#pragma once
#include <vector>
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"
#include "application/Entity/State/GameCharacterState.h"

//============================================================================
// ノードの種類
//============================================================================
enum class BehaviorNodeType {
    ACTION,       // 行動ノード（ステート遷移）
    CONDITION,    // 条件ノード
    SEQUENCE,     // シーケンスノード
    SELECTOR,     // セレクターノード
};

//============================================================================
// ノードデータ（JSONシリアライズ用）
//============================================================================
struct BehaviorNodeData {
    std::string name = "UnnamedNode";          // ノード名
    std::string nodeType = "ACTION";           // ノードタイプ（文字列）
    std::string targetState = "NONE";          // ACTION用: 遷移先ステート（文字列）
    std::string conditionType = "NONE";        // CONDITION用: 条件の種類（文字列）
    float conditionThreshold = 0.0f;           // CONDITION用: 閾値
    int scoreIndex = -1;                       // SCORE_ABOVE用: スコアのインデックス
    std::vector<BehaviorNodeData> children;    // 子ノード（SEQUENCE/SELECTOR用）
};

//============================================================================
// コンボデータ（1つのコンボセット）
//============================================================================
struct ComboData {
    std::string comboName = "UnnamedCombo";    // コンボ名
    BehaviorNodeData rootNode;                 // ルートノード
};

//============================================================================
// コンボセットデータ（複数のコンボをまとめたもの）
//============================================================================
struct ComboSetData {
    std::string setName = "DefaultComboSet";   // セット名
    std::vector<ComboData> combos;             // コンボリスト
    std::string rootType = "SELECTOR";         // ルートのコンポジットタイプ
};

// JSONシリアライズ用のマクロ
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BehaviorNodeData, name, nodeType, targetState, conditionType, conditionThreshold, scoreIndex, children)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComboData, comboName, rootNode)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ComboSetData, setName, combos, rootType)

//JSONディレクトリパスの定義
TAKEC_DEFINE_JSON_DIRECTORY_PATH(ComboSetData, "Resources/JsonLoader/ComboSet/");