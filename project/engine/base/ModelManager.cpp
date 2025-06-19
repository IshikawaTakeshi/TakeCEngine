#include "ModelManager.h"
#include "base/TextureManager.h"
#include "math/MatrixMath.h"

#include <cassert>

ModelManager* ModelManager::instance_ = nullptr;

ModelManager* ModelManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ModelManager();
	}
	return instance_;
}

void ModelManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	//ModelCommon初期化
	modelCommon_ = ModelCommon::GetInstance();
	modelCommon_->Initialize(dxCommon, srvManager);
}

void ModelManager::Finalize() {
	modelCommon_->Finalize();

	delete instance_;
	instance_ = nullptr;
}

void ModelManager::LoadModel(const std::string& modelDirectoryPath, const std::string& modelFile,const std::string& envMapFile) {

	//読み込み済みモデルの検索
	if (models_.contains(modelFile)) {
		//すでに読み込み済みならreturn
		return;
	}

	//モデルの生成とファイル読み込み、初期化
	ModelData* modelData = LoadModelFile(modelDirectoryPath, modelFile,envMapFile);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->Initialize(modelCommon_, modelData);

	//モデルをコンテナに追加
	models_.insert(std::make_pair(modelFile, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath) {

	//読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		//読み込みモデルを戻り値としてreturn
		return models_.at(filePath).get();
	}

	//ファイル名一致なし
	return nullptr;
}

std::unique_ptr<Model> ModelManager::CopyModel(const std::string& filePath) {
	
	//読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		//読み込みモデルを戻り値としてreturn
		std::unique_ptr<Model> model = std::make_unique<Model>();
		model->Initialize(modelCommon_, models_.at(filePath)->GetModelData());
		return model;
	}
	//ファイル名一致なし
	return nullptr;
}

//=============================================================================
// Modelファイルを読む関数
//=============================================================================

ModelData* ModelManager::LoadModelFile(const std::string& modelDirectoryPath, const std::string& modelFile,const std::string& envMapFile) {

	ModelData* modelData = new ModelData();
	Assimp::Importer importer;
	std::string filePath = "./Resources/" + modelDirectoryPath + "/" + modelFile;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	modelData->fileName = modelFile;
	modelData->mesh = std::make_unique<ModelMesh>();

	if (scene->HasMeshes()) {

		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;
		std::vector<VertexData> vertices = {};
		std::vector<uint32_t> indices = {};

		//Meshの解析
		for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
			aiMesh* mesh = scene->mMeshes[meshIndex];
			assert(mesh->HasNormals()); //法線がない場合は現在エラー

			
			
			modelData->mesh->GetSubMeshes().resize(scene->mNumMeshes);
			vertices.resize(mesh->mNumVertices);
			//Meshの頂点数の格納
			modelData->skinningInfoData.VertexCount = mesh->mNumVertices;

			//vertexの解析
			for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

				vertices[vertexIndex].position = { -position.x,position.y,position.z, 1.0f };
				vertices[vertexIndex].normal = { -normal.x,normal.y,normal.z };
				
				// UVがある場合は値を、無い場合は(0,0)を設定
				if (mesh->HasTextureCoords(0)) {
					vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
				} else {
					vertices[vertexIndex].texcoord = { 0.0f, 0.0f };
				}
			}
			//faceの解析
			for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				aiFace& face = mesh->mFaces[faceIndex];
				assert(face.mNumIndices == 3); //三角形以外はエラー

				//Indices解析
				for (uint32_t element = 0; element < face.mNumIndices; ++element) {
					uint32_t indicesIndex = face.mIndices[element];
					indices.push_back(indicesIndex);
				}
			}

			//頂点情報をまとめる
			modelData->mesh->GetAllVertices().insert(
				modelData->mesh->GetAllVertices().end(),
				vertices.begin(),
				vertices.end()
			);
			//インデックス情報をまとめる
			modelData->mesh->GetAllIndices().insert(
				modelData->mesh->GetAllIndices().end(),
				indices.begin(),
				indices.end()
			);
			modelData->mesh->GetSubMeshes()[meshIndex].vertexStart = vertexOffset;
			modelData->mesh->GetSubMeshes()[meshIndex].vertexCount = uint32_t(vertices.size());
			modelData->mesh->GetSubMeshes()[meshIndex].indexStart = indexOffset;
			modelData->mesh->GetSubMeshes()[meshIndex].indexCount = uint32_t(indices.size());

			//オフセットの更新
			vertexOffset += modelData->mesh->GetSubMeshes()[meshIndex].vertexCount;
			indexOffset += modelData->mesh->GetSubMeshes()[meshIndex].indexCount;

			//boneの解析
			//boneがない場合はスキップ
			if (mesh->HasBones() == false) {
				modelData->haveBone = false;
				continue;
			}
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				modelData->haveBone = true;
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

					//InverceBindPoseMatrixの作成
					jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
				}
			}

	
		}
	}

	//materialの解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials - 1; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		aiString aiTexturePath;

		modelData->mesh->GetSubMeshes()[materialIndex].material_ = std::make_unique<Material>();

		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexturePath) == AI_SUCCESS) {
			std::string textureFilePath = aiTexturePath.C_Str();


			if (!textureFilePath.empty() && textureFilePath[0] == '*') {
				//埋め込みテクスチャの場合
				int texIdx = std::stoi(textureFilePath.c_str() + 1);
				const aiTexture* embeddedTexture = scene->mTextures[texIdx];
				modelData->mesh->GetSubMeshes()[materialIndex].material_->SetTextureFilePath(embeddedTexture->mFilename.C_Str());
				TextureManager::GetInstance()->LoadEmbeddedTexture(embeddedTexture);
			} else {
				//外部テクスチャの場合
				modelData->mesh->GetSubMeshes()[materialIndex].material_->SetTextureFilePath(textureFilePath);
				TextureManager::GetInstance()->LoadTexture(textureFilePath);
			}
		}

		//テクスチャがない場合はデフォルトのテクスチャを設定
		if (modelData->mesh->GetSubMeshes()[materialIndex].material_->GetTextureFilePath() == "") { 
			modelData->mesh->GetSubMeshes()[materialIndex].material_->SetTextureFilePath("white1x1.png");
		}

		//環境マップテクスチャの設定
		//MEMO: 画像はDDSファイルのみ対応
		if (envMapFile != "") {
			assert(envMapFile.find(".dds") != std::string::npos); //DDSファイル以外はエラー)
			modelData->mesh->GetSubMeshes()[materialIndex].material_->SetEnvMapFilePath(envMapFile);
		}
	}

	//rootNodeの解析
	modelData->rootNode = ReadNode(scene->mRootNode);

	return modelData;
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
