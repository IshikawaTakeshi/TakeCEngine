#include "ModelManager.h"
#include "base/TextureManager.h"
#include "math/MatrixMath.h"
#include "Animation/SkinCluster.h"

#include <cassert>

ModelManager* ModelManager::instance_ = nullptr;

//==================================================================
// シングルトンインスタンス取得
//==================================================================
ModelManager* ModelManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new ModelManager();
	}
	return instance_;
}

//==================================================================
// 初期化
//==================================================================
void ModelManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	//ModelCommon初期化
	modelCommon_ = ModelCommon::GetInstance();
	modelCommon_->Initialize(dxCommon, srvManager);
}

//==================================================================
// 終了処理
//==================================================================
void ModelManager::Finalize() {
	modelCommon_->Finalize();

	delete instance_;
	instance_ = nullptr;
}

//==================================================================
// モデルの読み込み
//==================================================================
void ModelManager::LoadModel(const std::string& modelDirectoryPath, const std::string& modelFile, const std::string& envMapFile) {

	//読み込み済みモデルの検索
	if (models_.contains(modelFile)) {
		//すでに読み込み済みならreturn
		return;
	}

	//モデルの生成とファイル読み込み、初期化
	std::unique_ptr<ModelData> modelData = std::make_unique<ModelData>();
	modelData = std::move(LoadModelFile(modelDirectoryPath, modelFile, envMapFile));

	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->Initialize(modelCommon_, std::move(modelData));

	//モデルをコンテナに追加
	models_.insert(std::make_pair(modelFile, std::move(model)));
}

std::unique_ptr<Model> ModelManager::FindModel(const std::string& filePath) {

	//読み込み済みモデルを検索
	if (models_.contains(filePath)) {
		//読み込みモデルを戻り値としてreturn
		return models_[filePath]->Clone();
	}

	//ファイル名一致なし
	return nullptr;
}


//=============================================================================
// Modelファイルを読む関数
//=============================================================================
std::unique_ptr<ModelData> ModelManager::LoadModelFile(
	const std::string& modelDirectoryPath,
	const std::string& modelFile,
	const std::string& envMapFile) {

	auto modelData = std::make_unique<ModelData>();
	Assimp::Importer importer;

	// パス構築
	std::filesystem::path filePath = std::filesystem::path("./Resources") / modelDirectoryPath / modelFile;
	const aiScene* scene = importer.ReadFile(filePath.string(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	// エラーチェック
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
	}

	//ファイルネームの取得
	modelData->fileName = modelFile;

	// メッシュ＆ボーン・スキンクラスタ情報の読み込み
	LoadMeshesAndSkinCluster(scene, modelData.get());

	// マテリアル情報の読み込み
	LoadMaterials(scene, modelData.get(), envMapFile);

	//rootNodeの解析
	modelData->rootNode = ReadNode(scene->mRootNode);

	return std::move(modelData);
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

//========================================================================================
// 全メッシュ・全ボーン名, 頂点数の集計 
//========================================================================================
void ModelManager::LoadMeshesAndSkinCluster(const aiScene* scene, ModelData* modelData) {


	//全ての頂点数の合算値
	uint32_t totalVertices = 0;
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		totalVertices += scene->mMeshes[meshIndex]->mNumVertices;
	}

	//VertexInfluence配列を全頂点分初期化
	std::vector<VertexInfluence> vertexInfluences(totalVertices);

	//SubMeshのサイズ設定
	modelData->mesh.GetSubMeshes().resize(scene->mNumMeshes);
	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;

	if (scene->HasMeshes()) {

		//========================================================================
		// Meshの解析
		//========================================================================

		for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
			aiMesh* mesh = scene->mMeshes[meshIndex];
			assert(mesh->HasNormals()); //法線がない場合は現在エラー
			std::vector<VertexData> vertices(mesh->mNumVertices);
			std::vector<uint32_t> indices;

			//====================================================================
			//vertexの解析
			//====================================================================

			for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				vertices[vertexIndex].position = { -position.x,position.y,position.z, 1.0f };
				vertices[vertexIndex].normal = { -normal.x,normal.y,normal.z };

				// UVがある場合は値を、無い場合は(0,0)を設定
				if (mesh->HasTextureCoords(0)) {
					aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
					vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
				} else {
					vertices[vertexIndex].texcoord = { 0.0f, 0.0f };
				}
			}

			//=================================================================
			//indexの解析
			//=================================================================

			//faceの解析
			for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				aiFace& face = mesh->mFaces[faceIndex];
				assert(face.mNumIndices == 3); //三角形以外はエラー

				//Indices解析
				for (uint32_t element = 0; element < face.mNumIndices; ++element) {
					//MEMO: vertexOffsetを加算するとローカルのindicesがズレたので加算しないようにする
					indices.push_back(face.mIndices[element]);
				}
			}

			//=================================================================
			//全体頂点・インデックス情報を統合
			//=================================================================
			modelData->mesh.GetAllVertices().insert(
				modelData->mesh.GetAllVertices().end(),
				vertices.begin(), vertices.end()
			);
			modelData->mesh.GetAllIndices().insert(
				modelData->mesh.GetAllIndices().end(),
				indices.begin(), indices.end()
			);

			//=================================================================
			//SubMeshの情報を設定
			//=================================================================
			auto& subMesh = modelData->mesh.GetSubMeshes()[meshIndex];
			subMesh.vertexStart = vertexOffset;
			subMesh.vertexCount = uint32_t(vertices.size());
			subMesh.indexStart = indexOffset;
			subMesh.indexCount = uint32_t(indices.size());
			subMesh.vertices = std::move(vertices);
			subMesh.indices = std::move(indices);

			///////////////////////////////////////////////////////////////////////
			// bone・skinClusterの解析
			///////////////////////////////////////////////////////////////////////
			if (mesh->HasBones()) {

				modelData->haveBone = true;

				//======================================================================
				// boneの解析
				//======================================================================
				for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
					aiBone* bone = mesh->mBones[boneIndex];
					std::string boneName = bone->mName.C_Str();
					JointWeightData& jointWeightData = modelData->skinClusterData[boneName];

					// InverseBindPoseMatrixをセット（初回のみ）
					if (jointWeightData.inverseBindPoseMatrix == Matrix4x4{}) {
						//BindPoseMatrixに戻す
						aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
						aiVector3D scale, translate;
						aiQuaternion rotate;

						//成分の抽出
						bindPoseMatrixAssimp.Decompose(scale, rotate, translate);

						//左手系のBindPoseMatrixの作成
						Matrix4x4 bindPoseMatrix = MatrixMath::MakeAffineMatrix(
							{ scale.x,scale.y,scale.z },
							{ rotate.x,-rotate.y,-rotate.z,rotate.w },
							{ -translate.x,translate.y,translate.z });
						jointWeightData.inverseBindPoseMatrix = MatrixMath::Inverse(bindPoseMatrix);
					}

					//==================================================================
					// Weightの解析
					//==================================================================
					for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
						uint32_t meshLocalVertexIdx = bone->mWeights[weightIndex].mVertexId;
						float weight = bone->mWeights[weightIndex].mWeight;
						uint32_t globalVertexIdx = meshLocalVertexIdx + vertexOffset; //全体頂点数に合わせてインデックスを調整
						//InverceBindPoseMatrixの作成
						jointWeightData.vertexWeights.push_back({weight,globalVertexIdx});

						// VertexInfluenceの設定
						VertexInfluence& influence = vertexInfluences[globalVertexIdx];
						for (uint32_t i = 0; i < kNumMaxInfluence; ++i) {
							//Weightが0.0fの場合、WeightとJointのIndexを書き込む
							if (influence.weights[i] == 0.0f) {
								influence.weights[i] = weight;
								influence.jointIndices[i] = boneIndex; // boneIndexは0から始まるのでそのまま使用
								break;
							}
						}
					}
				}
			}

			//=================================================================
			//オフセットの更新
			//=================================================================
			vertexOffset += modelData->mesh.GetSubMeshes()[meshIndex].vertexCount;
			indexOffset += modelData->mesh.GetSubMeshes()[meshIndex].indexCount;
		}

		//全体頂点数の設定
		modelData->skinningInfoData.VertexCount = totalVertices;

		//influenceをModelDataに保存
		modelData->influences = std::move(vertexInfluences);
	}
}

//=============================================================================4
// マテリアルの読み込み
//=============================================================================
void ModelManager::LoadMaterials(const aiScene* scene, ModelData* modelData, const std::string& envMapFile) {

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials - 1; ++materialIndex) {
		if (materialIndex >= scene->mNumMeshes) break; // 境界チェック
		aiMaterial* material = scene->mMaterials[materialIndex];
		aiString aiTexturePath;

		// マテリアル名の設定
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexturePath) == AI_SUCCESS) {
			std::string textureFilePath = aiTexturePath.C_Str();
			if (!textureFilePath.empty() && textureFilePath[0] == '*') {
				// 埋め込みテクスチャの場合の処理
				try {
					int texIdx = std::stoi(textureFilePath.c_str() + 1);
					if (texIdx >= 0 && texIdx < static_cast<int>(scene->mNumTextures)) {
						const aiTexture* embeddedTexture = scene->mTextures[texIdx];
						modelData->mesh.GetSubMeshes()[materialIndex].material_.SetTextureFilePath(embeddedTexture->mFilename.C_Str());
						TextureManager::GetInstance()->LoadEmbeddedTexture(embeddedTexture);
					}
				}
				catch (const std::exception&) {
					// 変換エラーの場合はデフォルトテクスチャを使用
					modelData->mesh.GetSubMeshes()[materialIndex].material_.SetTextureFilePath("white1x1.png");
				}

			} else {
				//外部テクスチャの場合
				modelData->mesh.GetSubMeshes()[materialIndex].material_.SetTextureFilePath(textureFilePath);
				TextureManager::GetInstance()->LoadTexture(textureFilePath);
			}
		}

		//テクスチャがない場合はデフォルトのテクスチャを設定
		if (modelData->mesh.GetSubMeshes()[materialIndex].material_.GetTextureFilePath().empty()) {
			modelData->mesh.GetSubMeshes()[materialIndex].material_.SetTextureFilePath("uvChecker.png");
		}

		//環境マップテクスチャの設定
		//MEMO: 画像はDDSファイルのみ対応
		if (!envMapFile.empty()) {
			assert(envMapFile.find(".dds") != std::string::npos); //DDSファイル以外はエラー)
			modelData->mesh.GetSubMeshes()[materialIndex].material_.SetEnvMapFilePath(envMapFile);
		}

		//マテリアルの初期化
		modelData->mesh.GetSubMeshes()[materialIndex].material_.Initialize(modelCommon_->GetDirectXCommon(), modelData->mesh.GetSubMeshes()[materialIndex].material_.GetTextureFilePath(), modelData->mesh.GetSubMeshes()[materialIndex].material_.GetEnvMapFilePath());
	}
}
