#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <string>
#include <vector>
#include <cstdint>

struct MaterialData {
	Vector4 color; //カラー
	uint32_t enableLighting; //ライティングを有効にするフラグ
	float padding[3];
	Matrix4x4 uvTransform; //UVトランスフォーム
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct DirectionalLightData {
	Vector4 color_; //ライトの色
	Vector3 direction_; //ライトの向き
	float intensity_; //輝度
};


struct ModelMaterialData {

	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

struct ModelData {

	std::vector<VertexData> vertices;
	ModelMaterialData material;
};