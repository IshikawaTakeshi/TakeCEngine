#pragma once
#include "Vector3.h"
#include <json.hpp>
#include <numbers>
#include <cstdint>

struct AttributeRange {
	float min;
	float max;
};

enum class ScaleSetting {
	None = 0, //スケールの更新なし
	ScaleUp = 1, //スケールの更新(拡大)
	ScaleDown = 2, //スケールの更新(縮小)
};

// パーティクルの属性を保持する構造体
struct ParticleAttributes {
	Vector3 scale = { 1.0f,1.0f,1.0f };
	Vector3 color = { 1.0f,1.0f,1.0f };
	AttributeRange scaleRange = { 0.1f,3.0f };
	AttributeRange rotateRange = { -std::numbers::pi_v<float>, std::numbers::pi_v<float> };
	AttributeRange angleRange = { 0.0f, 0.0f };
	AttributeRange positionRange = {-1.0f, 1.0f};
	AttributeRange velocityRange = { -1.0f,1.0f };
	AttributeRange colorRange = { 0.0f,1.0f };
	AttributeRange lifetimeRange = { 1.0f,3.0f };
	float frequency; //パーティクルの発生頻度
	uint32_t emitCount; //1回あたりのパーティクル発生数
	bool isBillboard = false; //Billboardかどうか
	bool editColor = false; //色を編集するかどうか
	bool isTraslate_ = false; //位置を更新するかどうか
	uint32_t scaleSetting_;    //スケールの更新処理方法
	bool enableFollowEmitter_ = false; //エミッターに追従するかどうか
};

using json = nlohmann::json;

// JSON形式に変換(ParticleAttributes)
void to_json(json& j, const ParticleAttributes& attributes);

// JSON形式に変換(AttributeRange)
void to_json(json& j, const AttributeRange& attributeRange);

void from_json(const json& j, ParticleAttributes& attributes);

void from_json(const json& j, AttributeRange& attributeRange);