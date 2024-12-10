#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include <string>
#include <vector>
#include <cstdint>



struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//DirectionalLightのデータ
struct DirectionalLightData {
	Vector4 color_; //ライトの色
	Vector3 direction_; //ライトの向き
	float intensity_; //輝度
};

//PointLightのデータ
struct PointLightData {
	Vector4 color_; //ライトの色
	Vector3 position_; //ライトの位置
	float intensity_; //輝度
	float radius_; //影響範囲
	float decay_; //減衰率
	float padding[2];
};

//SpotLightのデータ
struct SpotLightData {
	Vector4 color_;       //ライトの色
	Vector3 position_;    //ライトの位置
	float intensity_;     //輝度
	Vector3 direction_;   //ライトの向き
	float distance_;      //影響範囲
	float decay_;         //減衰率
	float cosAngle_;     //スポットライトの角度
	float penumbraAngle_; //影のぼかし角度
};

//モデル1個分のマテリアルデータ
struct ModelMaterialData {

	std::string textureFilePath; //テクスチャファイルのパス
	uint32_t srvIndex; //テクスチャのインデックス
};

struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

//モデル1個分のデータ
struct ModelData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	ModelMaterialData material;
	Node rootNode;
};

//パーティクル用の行列,色データ
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 Color;
};