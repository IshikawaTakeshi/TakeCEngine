#pragma once
#include "engine/math/Vector2.h"
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"
#include "engine/math/Transform.h"
#include "engine/math/Matrix4x4.h"
#include "engine/3d/VertexData.h"
#include "engine/3d/Mesh/SubMesh.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>


//モデル1個分のマテリアルデータ
struct ModelMaterialData {

	std::string textureFilePath; //テクスチャファイルのパス
	std::string envMapFilePath; //環境マップのパス
	uint32_t textureIndex; //テクスチャのインデックス

	Vector4 baseColor = { 1.0f,1.0f,1.0f,1.0f }; //ベースカラー(ディフューズカラー)
	EulerTransform uvTransform; //UVトランスフォーム
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
	std::vector<VertexData> vertices; //モデルの全ての頂点データ
	SkinningInfo skinningInfoData; //スキニング情報
	std::vector<uint32_t> indices; //モデルの全ての頂点インデックス
	Node rootNode;
	bool haveBone = false;

	ModelMaterialData material;
	//複数マテリアル
	std::vector<ModelMaterialData> materials;
	// 互換用: 0番マテリアルへのアクセサとして残す
	std::vector<SubMesh> subMeshes;   // 追加: サブメッシュリスト
};