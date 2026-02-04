#pragma once
#include <string>
#include <vector>
#include <memory>
#include <json.hpp>
#include "engine/math/Vector3.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include "engine/3d/Particle/ParticleEmitter.h"

//============================================================================
// EmitterConfig struct - エミッター個別の設定
//============================================================================
struct EmitterConfig {
	std::string emitterName;           // エミッター名
	std::string presetFilePath;        // プリセットJSONファイルパス
	Vector3 positionOffset = {0, 0, 0}; // エフェクト中心からのオフセット
	Vector3 rotationOffset = {0, 0, 0}; // 回転オフセット
	float delayTime = 0.0f;             // 発生遅延時間（秒）
	float duration = -1.0f;             // 持続時間（-1 = 無限）
	bool autoStart = true;              // 自動開始フラグ
	uint32_t emitCount = 1;             // 発生させる回数（0 = 連続）
};

//============================================================================
// EffectGroupConfig struct - エフェクトグループ全体の設定
//============================================================================
struct EffectGroupConfig {
	std::string effectName;                    // エフェクト名
	std::vector<EmitterConfig> emitters;       // エミッター設定のリスト
	bool isLooping = false;                    // ループ再生フラグ
	float totalDuration = -1.0f;               // エフェクト全体の持続時間
	Vector3 defaultScale = {1.0f, 1.0f, 1.0f}; // デフォルトスケール
};

//============================================================================
// EmitterInstance struct - 実行時のエミッターインスタンス
//============================================================================
struct EmitterInstance {
	std::unique_ptr<ParticleEmitter> emitter; // エミッター本体
	EmitterConfig config;                      // 設定
	float elapsedTime = 0.0f;                  // 経過時間
	uint32_t emitCounter = 0;                  // 発生カウンター
	bool isActive = false;                     // アクティブフラグ
	bool hasStarted = false;                   // 開始済みフラグ
};

// エミッター設定のJSONシリアライズ定義
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EmitterConfig,
	emitterName,
	presetFilePath,
	positionOffset,
	rotationOffset,
	delayTime,
	duration,
	autoStart,
	emitCount
)

// エフェクトグループ設定のJSONシリアライズ定義
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectGroupConfig,
	effectName,
	emitters,
	isLooping,
	totalDuration,
	defaultScale
)

// JSONディレクトリパスの定義
TAKEC_DEFINE_JSON_DIRECTORY_PATH(EffectGroupConfig, "Resources/JsonLoader/EffectGroup/");