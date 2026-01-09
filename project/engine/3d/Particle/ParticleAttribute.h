#pragma once
#include "engine/math/Vector3.h"
#include "engine/3d/Primitive/PrimitiveType.h"
#include "engine/base/BlendModeStateEnum.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include <json.hpp>
#include <numbers>
#include <string>
#include <cstdint>
#include <unordered_map>

//============================================================================
// ParticleAttribute.h
//============================================================================

// パーティクルモデルの種類を表す列挙型
enum class ParticleModelType {
	Primitive,
	ExternalModel
};

// 属性の範囲を表す構造体
struct AttributeRange {
	float min;
	float max;
};

// スケール設定の種類を表す列挙型
enum class ScaleSetting {
	None = 0, //スケールの更新なし
	ScaleUp = 1, //スケールの更新(拡大)
	ScaleDown = 2, //スケールの更新(縮小)
};

// パーティクルの属性を保持する構造体
struct ParticleAttributes {
	Vector3 scale = { 1.0f,1.0f,1.0f };
	Vector3 color = { 1.0f,1.0f,1.0f };
	Vector3 direction = { 0.0f,0.0f,0.0f };
	AttributeRange scaleRange = { 0.1f,3.0f };
	AttributeRange rotateRange = { -std::numbers::pi_v<float>, std::numbers::pi_v<float> };
	AttributeRange positionRange = {-1.0f, 1.0f};
	AttributeRange velocityRange = { -1.0f,1.0f };
	AttributeRange colorRange = { 0.0f,1.0f };
	AttributeRange lifetimeRange = { 1.0f,3.0f };
	float frequency = 0.1f; //パーティクルの発生頻度
	uint32_t emitCount = 1; //1回あたりのパーティクル発生数
	bool isBillboard = false; //Billboardかどうか
	bool editColor = false; //色を編集するかどうか
	bool isTranslate = false; //位置を更新するかどうか
	bool isDirectional = false; //方向に沿って移動するかどうか
	uint32_t scaleSetting = 0;    //スケールの更新処理方法
	bool enableFollowEmitter = false; //エミッターに追従するかどうか

	bool isParticleTrail = false; //パーティクルによるトレイルを有効にするかどうか
	uint32_t particlesPerInterpolation = 5; //一度の補間で生成するパーティクル数
	float trailEmitInterval = 0.016f; //トレイルエフェクトの生成間隔

	bool isEmitterTrail = false; //エミッターの移動に応じてトレイルを生成するかどうか
};

// パーティクルプリセットを保持する構造体
struct ParticlePreset {
	std::string presetName; //プリセットの名前
	ParticleAttributes attribute; //パーティクルの属性情報
	BlendState blendState = BlendState::ADD;
	std::string textureFilePath; //テクスチャファイル名
	PrimitiveType primitiveType; //プリミティブの種類
	Vector3 primitiveParameters = {1.0f,1.0f,1.0f}; //プリミティブのパラメータ
};

// nlohmann::jsonのエイリアス
using json = nlohmann::json;

// JSON形式に変換
void to_json(json& j, const ParticleAttributes& attributes);
void to_json(json& j, const AttributeRange& attributeRange);
void to_json(json& j, const ParticlePreset& preset);

// JSONから各データ構造体に変換
void from_json(const json& j, ParticleAttributes& attributes);
void from_json(const json& j, AttributeRange& attributeRange);
void from_json(const json& j, ParticlePreset& preset);

//ディレクトリパス設定
TAKEC_DEFINE_JSON_DIRECTORY_PATH(ParticlePreset, "Resources/JsonLoader/ParticlePresets/");
