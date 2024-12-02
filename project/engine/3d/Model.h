#pragma once

#include "Mesh.h"
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <d3d12.h>
#include <wrl.h>
#include <memory>

class Texture;
class SrvManager;
class DirectXCommon;
class ModelCommon;
class Model {
public:

	Model() = default;
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* ModelCommon, const std::string& modelDirectoryPath, const std::string& filename);

	/// <summary>
	/// 更新処理
	/// </summary>
	//void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void DrawForParticle(UINT instanceCount_);

	/// <summary>
	/// objファイルを読む関数
	/// </summary>
	ModelData LoadModelFile(const std::string& DirectoryPath, const std::string& filename);
	
	/// <summary>
	/// mtlファイルを読む関数
	/// </summary>
	ModelMaterialData LoadMtlFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);

	Node ReadNode(aiNode* node);

public: //ゲッター

	Mesh* GetMesh() { return mesh_.get(); }

	ModelData& GetModelData() { return modelData_; }

	ModelCommon* GetModelCommon() { return modelCommon_; }

	const std::string& GetTextureFilePath() const { return modelData_.material.textureFilePath; }

public: //セッター

	void SetMesh(Mesh* mesh) { mesh_.reset(mesh); }

	void SetModelCommon(ModelCommon* modelCommon) { modelCommon_ = modelCommon; }

private:

	ModelCommon* modelCommon_ = nullptr;

	//メッシュ
	std::unique_ptr<Mesh> mesh_ = nullptr;

	//構築するModelData
	ModelData modelData_;
};