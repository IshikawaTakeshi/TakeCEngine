#pragma once
#include "ResourceDataStructure.h"

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>
#include <string>
#include <memory>

//前方宣言
class Model;
class ModelCommon;
class SrvManager;
class DirectXCommon;

//============================================================================
// ModelManager class
//============================================================================
class ModelManager {
private:

	//コピーコンストラクタ・代入演算子禁止
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

public:

	//========================================================================
	// functions
	//========================================================================
	
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
	Model* FindModel(const std::string& filePath);

	/// <summary>
	/// モデルのコピーを作成する関数
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns></returns>
	std::unique_ptr<Model> CopyModel(const std::string& filePath);

	/// <summary>
	/// modelファイルを読む関数
	/// </summary>
	ModelData* LoadModelFile(const std::string& DirectoryPath, const std::string& modelFile,const std::string& envMapFile);

	/// <summary>
	/// ノードの読み込み
	/// </summary>
	Node ReadNode(aiNode* rootNode);


private:

	//インスタンス
	static ModelManager* instance_;

	//モデル共通データ
	ModelCommon* modelCommon_ = nullptr;

	//モデルデータコンテナ
	std::map<std::string, std::shared_ptr<Model>> models_;
};

