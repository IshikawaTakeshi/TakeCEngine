#include "ModelManager.h"
#include "Model.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "MatrixMath.h"

#include <cassert>

//シングルトンインスタンスの初期化
ModelManager* ModelManager::instance_ = nullptr;

//=============================================================================
// シングルトンインスタンスの取得
//=============================================================================
ModelManager* ModelManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ModelManager();
	}
	return instance_;
}

//=============================================================================
// 初期化
//=============================================================================
void ModelManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	//ModelCommon初期化
	modelCommon_ = ModelCommon::GetInstance();
	modelCommon_->Initialize(dxCommon, srvManager);
}

//=============================================================================
// 終了処理
//=============================================================================
void ModelManager::Finalize() {
	modelCommon_->Finalize();

	delete instance_;
	instance_ = nullptr;
}

//=============================================================================
// モデルの読み込み
//=============================================================================
void ModelManager::LoadModel(const std::string& modelFile,const std::string& envMapFile) {

	//読み込み済みモデルの検索
	if (models_.contains(modelFile)) {
		//すでに読み込み済みならreturn
		return;
	}

	//モデルの生成とファイル読み込み、初期化
	ModelData* modelData = LoadModelFile(modelFile,envMapFile);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->Initialize(modelCommon_, modelData);

	//モデルをコンテナに追加
	models_.insert({ modelFile, std::move(model) });
	// モデルインスタンスを登録
	RegisterInstance(modelFile, models_.at(modelFile));
}

//=============================================================================
// モデルの検索
//=============================================================================
Model* ModelManager::FindModel(const std::string& filePath) {

	//読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		//読み込みモデルを戻り値としてreturn
		return models_.at(filePath).get();
	}

	//ファイル名一致なし
	return nullptr;
}

//=============================================================================
// モデルのコピーを作成
//=============================================================================
std::shared_ptr<Model> ModelManager::CopyModel(const std::string& filePath) {
	
	auto it = models_.find(filePath);
	if (it == models_.end()) {
		return nullptr;
	}

	//モデルの生成と初期化
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->Initialize(modelCommon_, it->second->GetModelData());

	RegisterInstance(filePath, model);
	return model;

}

//=============================================================================
// Modelファイルを読む関数
//=============================================================================

ModelData* ModelManager::LoadModelFile(const std::string& modelFile,const std::string& envMapFile) {

	namespace fs = std::filesystem;

	// 拡張子から検索ルートを決める
	fs::path fileNamePath(modelFile);
	std::string ext = fileNamePath.extension().string();

	// ベースディレクトリ
	fs::path baseDir = "./Resources/Models/";

	// 拡張子ごとの検索ディレクトリを決める
	fs::path modelDir;
	if (ext == ".fbx") {
		modelDir = baseDir / "fbx/";
	} else if (ext == ".obj") {
		modelDir = baseDir / "obj/";
	} else if (ext == ".gltf" || ext == ".glb") {
		modelDir = baseDir / "gltf/";
	}

	// 最終的なフルパス
	fs::path fullPath = modelDir / modelFile;

	ModelData* modelData = new ModelData();
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fullPath.string().c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	modelData->fileName = modelFile;

	if (scene->HasMeshes()) {

		modelData->vertices.clear();
		modelData->indices.clear();
		modelData->subMeshes.clear();
		modelData->materials.clear();

		uint32_t globalVertexOffset = 0;
		uint32_t globalIndexOffset  = 0;

		modelData->haveBone = false;

		//Meshの解析
		for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
			aiMesh* mesh = scene->mMeshes[meshIndex];
			SubMesh sub{};
			sub.vertexStart = globalVertexOffset;
			sub.indexStart  = globalIndexOffset;
			sub.vertexCount = mesh->mNumVertices;
			sub.materialIndex = mesh->mMaterialIndex; // aiMeshが参照するマテリアル番号を保持
			assert(mesh->HasNormals()); //法線がない場合は現在エラー
			

			modelData->vertices.resize(mesh->mNumVertices);
			//Meshの頂点数の格納
			modelData->skinningInfoData.numVertices = mesh->mNumVertices;

			//vertexの解析
			for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

				modelData->vertices[vertexIndex].position = { -position.x,position.y,position.z, 1.0f };
				modelData->vertices[vertexIndex].normal = { -normal.x,normal.y,normal.z };
				
				// UVがある場合は値を、無い場合は(0,0)を設定
				if (mesh->HasTextureCoords(0)) {
					modelData->vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
				} else {
					modelData->vertices[vertexIndex].texcoord = { 0.0f, 0.0f };
				}
			}
			
			//faceの解析
			for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				aiFace& face = mesh->mFaces[faceIndex];
				assert(face.mNumIndices == 3); //三角形以外はエラー

				//Indices解析
				for (uint32_t element = 0; element < face.mNumIndices; ++element) {
					uint32_t localIndex = face.mIndices[element];
					uint32_t globalIndex = globalVertexOffset + localIndex;
					modelData->indices.push_back(globalIndex);
				}
			}

			// サブメッシュのインデックス数の計算
			sub.indexCount = uint32_t(modelData->indices.size()) - sub.indexStart;

			modelData->subMeshes.push_back(sub);

			//globalOffsetに加算
			globalVertexOffset += mesh->mNumVertices;
			globalIndexOffset  += sub.indexCount;

			//----------------------------------------------------
			//boneの解析
			//----------------------------------------------------
			//boneがない場合はスキップ
			if (mesh->HasBones() == false) {
				continue;
			} else {
				modelData->haveBone = true;
			}

			
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				aiBone* bone = mesh->mBones[boneIndex];
				std::string jointwName = bone->mName.C_Str();
				JointWeightData& jointWeightData = modelData->skinClusterData[jointwName];

				//BindPoseMatrixに戻す
				aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
				aiVector3D scale, translate;	
				aiQuaternion rotate;

				//成分の抽出
				bindPoseMatrixAssimp.Decompose(scale, rotate, translate);

				//左手系のBindPoseMatrixの作成
				Matrix4x4 bindPoseMatrix = MatrixMath::MakeAffineMatrix(
					{ scale.x,scale.y,scale.z }, { rotate.x,-rotate.y,-rotate.z,rotate.w }, { -translate.x,translate.y,translate.z });
				jointWeightData.inverseBindPoseMatrix = MatrixMath::Inverse(bindPoseMatrix);

				//Weightの解析
				for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {

					//InverseBindPoseMatrixの作成
					jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
				}
			}
		}
	}

	//------------------------------------------------
	//materialの解析
	//------------------------------------------------

	modelData->materials.clear();
	modelData->materials.resize(scene->mNumMaterials);

	// 
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		ModelMaterialData& mat = modelData->materials[materialIndex];

		aiString textureFilePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
			mat.textureFilePath = textureFilePath.C_Str();
		}

		if (mat.textureFilePath == "") { //テクスチャがない場合はデフォルトのテクスチャを設定
			mat.textureFilePath = "white1x1.png";
		}

		//環境マップテクスチャの設定
		//MEMO: 画像はDDSファイルのみ対応
		if (!envMapFile.empty()) {
			assert(envMapFile.find(".dds") != std::string::npos);
			mat.envMapFilePath = envMapFile;
		} else {
			mat.envMapFilePath = "rostock_laage_airport_4k.dds"; // 旧仕様を踏襲するか要検討
		}
	}

	//rootNodeの解析
	modelData->rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

//=============================================================================
// モデルのリロード
//=============================================================================
void ModelManager::ReloadModel(const std::string& modelFile) {

	const std::string& fileKey = modelFile;

	ModelData* newData = LoadModelFile(fileKey, models_.at(fileKey)->GetModelData()->material.envMapFilePath);


	// このモデルファイルを使っている全インスタンスに通知
	auto it = modelInstances_.find(fileKey);
	if (it != modelInstances_.end()) {
		auto& vec = it->second;
		for (auto iter = vec.begin(); iter != vec.end(); ) {
			if (auto instance = iter->lock()) {
				// 生きているインスタンスなら Reload
				instance->Reload(newData);
				++iter;
			} else {
				// すでに破棄されたインスタンスはリストから消してクリーンアップ
				iter = vec.erase(iter);
			}
		}
	}

	// 元のモデルにも通知
	if (auto itRep = models_.find(fileKey); itRep != models_.end()) {
		itRep->second->Reload(newData);
	}
}

//=============================================================================
// Nodeの解析
//=============================================================================

Node ModelManager::ReadNode(aiNode* node) {
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w }; //x軸を反転
	result.transform.translate = { -translate.x,translate.y,translate.z }; //x軸を反転
	result.localMatrix = MatrixMath::MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	result.name = node->mName.C_Str(); //名前の格納
	result.children.resize(node->mNumChildren); //子ノードの数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]); //再帰的に子ノードを読む
	}
	return result;
}

void ModelManager::RegisterInstance(const std::string& filePath, std::shared_ptr<Model> modelInstance) {
	//ファイルパスをキーにモデルインスタンスを登録
	modelInstances_[filePath].push_back(modelInstance);
}

void ModelManager::RequestReload(const std::string& modelFile) {

	reloadRequests_.push_back(modelFile);
}

void ModelManager::ApplyModelReloads() {
	for (const std::string& key : reloadRequests_) {
		ReloadModel(key);   // ここで ModelData 読み直し + 全 Model::Reload
	}
	reloadRequests_.clear();
}
