#pragma once
#include "ResourceDataStructure.h"
#include "3d/Model.h"
#include "3d/ModelCommon.h"
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>
#include <string>
#include <memory>

class ModelManager {
public:
	
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static ModelManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	/// <summary>
	/// 開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// モデルの読み込み
	/// </summary>
	/// <param name="filePath">モデルのファイルパス</param>
	void LoadModel(const std::string& modelDirectoryPath, const std::string& modelFile,const std::string& envMapFile = "rostock_laage_airport_4k.dds");

	/// <summary>
	/// モデルの検索
	/// </summary>
	/// <param name="filePath">モデルのファイルパス</param>
	std::unique_ptr<Model> FindModel(const std::string& filePath);

	/// <summary>
	/// modelファイルを読む関数
	/// </summary>
	std::unique_ptr<ModelData> LoadModelFile(const std::string& DirectoryPath, const std::string& modelFile,const std::string& envMapFile);

private:

	/// <summary>
	/// ノードの読み込み
	/// </summary>
	Node ReadNode(aiNode* rootNode);

	// メッシュ・頂点・インデックス情報の読み込み
	void LoadMeshesAndSkinCluster(const aiScene* scene, ModelData* modelData);

	// マテリアル・テクスチャ情報の読み込み
	void LoadMaterials(const aiScene* scene, ModelData* modelData, const std::string& envMapFile);
	
private:

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

private:

	//インスタンス
	static ModelManager* instance_;

	ModelCommon* modelCommon_ = nullptr;

	//モデルデータコンテナ
	std::map<std::string, std::shared_ptr<Model>> models_;
};