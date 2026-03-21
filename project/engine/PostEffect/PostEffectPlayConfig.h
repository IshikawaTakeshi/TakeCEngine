#pragma once
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"
#include <string>

//============================================================
// PostEffectPlayConfig struct
// PostEffect 一時再生リクエストのパラメータ。JSONで保存・読み込みする。
//============================================================
struct PostEffectPlayConfig {
    // 対象エフェクト名 (例: "Vignette")
    std::string effectName;
    // プリセット識別名 (例: "DamageFlash")
    std::string presetName;
    // 再生時間（秒）
    float duration = 1.0f;
    // イージングタイプ（Easing::EasingType を int として保存）
    int easingType = 0;
    // 開始・ピーク・終了それぞれの強度
    float startIntensity = 0.0f;
    float peakIntensity  = 1.0f;
    float endIntensity   = 0.0f;
    // ピークに達する時間の割合（0.0~1.0）
    float peakTimeRate   = 0.5f;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PostEffectPlayConfig,
    effectName,
    presetName,
    duration,
    easingType,
    startIntensity,
    peakIntensity,
    endIntensity,
    peakTimeRate
)

// JSONディレクトリ
TAKEC_DEFINE_JSON_DIRECTORY_PATH(PostEffectPlayConfig, "Resources/JsonLoader/PostEffectPresets/");
