#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <wrl.h>

//頂点データ構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//モデル1個分のマテリアルデータ
struct ModelMaterialData {

	std::string textureFilePath; //テクスチャファイルのパス
	std::string envMapFilePath; //環境マップのパス
	uint32_t srvIndex; //テクスチャのインデックス
};

//アニメーションノード構造体
struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

//頂点ウェイトデータ構造体
struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

//ジョイントウェイトデータ構造体
struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

//スキニング情報構造体
struct SkinningInfo {
	uint32_t numVertices;
};

//モデル1個分のデータ
struct ModelData {
	std::string fileName; //モデル名
	std::map<std::string, JointWeightData> skinClusterData;
	std::vector<VertexData> vertices;
	SkinningInfo skinningInfoData;
	std::vector<uint32_t> indices;
	ModelMaterialData material;
	Node rootNode;
	bool haveBone = false;
};

//パーティクル用の行列,色データ
struct ParticleForGPU {
	Vector3 translate;
	Vector3 rotate;
	Vector3 scale;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

//フレーム情報
struct PerFrame {
	//ゲームを開始してからの時間
	float gameTime;
	//フレームの経過時間
	float deltaTime;
};