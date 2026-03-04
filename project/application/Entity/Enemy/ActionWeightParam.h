#pragma once
#include "engine/Utility/JsonDirectoryPathData.h"
#include "engine/Utility/JsonLoader.h"

//============================================================================
// ActionWeightParam struct
//============================================================================

struct AttackWeightParam {
    float distanceWeight = 0.8f;       // 距離係数への乗数
    float weaponChooseThreshold = 0.5f; // 武器選択の閾値
};

struct StepBoostWeightParam {
    float dangerFactorHigh = 0.7f;    // 弾が近い時の危険度
    float dangerFactorLow = 0.05f;   // 弾が遠い時の危険度
    float urgencyWeight = 1.5f;    // HP低下時の緊急度乗数
};

struct JumpWeightParam {
    float heightWeight = 0.3f;   // 高度差スコアへの乗数
    float energyWeight = 0.15f;  // エネルギー係数への乗数
};

struct FloatingWeightParam {
    float finalScoreWeight = 0.9f; // 最終スコアの乗数
    float noObstacleFactor = 0.2f; // 障害物なし時のobstacleFactor
};

struct ActionWeightParam {
    AttackWeightParam    attack;
    StepBoostWeightParam stepBoost;
    JumpWeightParam      jump;
    FloatingWeightParam  floating;
};

// JSONへの変換関数
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AttackWeightParam, distanceWeight, weaponChooseThreshold)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StepBoostWeightParam, dangerFactorHigh, dangerFactorLow, urgencyWeight)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JumpWeightParam, heightWeight, energyWeight)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FloatingWeightParam, finalScoreWeight, noObstacleFactor)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ActionWeightParam, attack, stepBoost, jump, floating)

// JSONディレクトリパスの定義
TAKEC_DEFINE_JSON_DIRECTORY_PATH(ActionWeightParam, "GameCharacterData/ActionWeightParam/");