#include "Model.h"
#include "DirectXCommon.h"
#include "Material.h"
#include "MatrixMath.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelCommon.h"

#include <fstream>
#include <sstream>
#include <cassert>



Model::~Model() {

	mesh_.reset();
}

//=============================================================================
// 初期化
//=============================================================================

void Model::Initialize(ModelCommon* ModelCommon, const std::string& modelDirectoryPath, const std::string& filePath) {
	modelCommon_ = ModelCommon;

	//objファイル読み込み
	modelData_ = LoadModelFile(modelDirectoryPath, filePath);
	//Animation読み込み
	animation_ = Animation::LoadAnimationFile(modelDirectoryPath, filePath);
	//Skeleton作成
	skeleton_ = CreateSkeleton(modelData_.rootNode);

	//メッシュ初期化
	mesh_ = std::make_unique<Mesh>();
	mesh_->InitializeMesh(modelCommon_->GetDirectXCommon(), modelData_.material.textureFilePath);
	
	//VertexResource
	mesh_->InitializeVertexResourceObjModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//MaterialResource
	mesh_->GetMaterial()->InitializeMaterialResource(modelCommon_->GetDirectXCommon()->GetDevice());
}

//=============================================================================
// 更新処理
//=============================================================================

void Model::Update() {

	//アニメーションがない場合は何もしない
	if (animation_.GetDuration() == 0.0f) {
		return;
	}
	
	//60fpsで進める
	//MEMO: 計測した時間を使って可変フレーム対応するのが望ましい
	animationTime += 1.0f / 60.0f;

	//アニメーションの更新とボーンへの適用
	ApplyAnimation();
	UpdateSkeleton();

	//最後まで行ったら最初からリピート再生する
	animationTime = std::fmod(animationTime, animation_.GetDuration());

	//rootNodeのAnimationを取得
	NodeAnimation& rootNodeAnimation = animation_.GetNodeAnimation(modelData_.rootNode.name);
	translate_ = Animation::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
	rotate_ = Animation::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
	scale_ = Animation::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
	localMatrix_ = MatrixMath::MakeAffineMatrix(scale_, rotate_, translate_);
}

//=============================================================================
// スケルトンの更新
//=============================================================================

void Model::UpdateSkeleton() {
	//全てのJointを更新
	for (Joint& joint : skeleton_.joints) {
		joint.localMatrix = MatrixMath::MakeAffineMatrix(
			joint.transform.scale, 
			joint.transform.rotate,
			joint.transform.translate);
		if (joint.parent) { //親がいる場合親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton_.joints[*joint.parent].skeletonSpaceMatrix;
		} else { //親がいない場合は自身の行列をそのまま使う
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================

void Model::Draw() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView()); // VBVを設定
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

//=============================================================================
// パーティクル用描画処理
//=============================================================================

void Model::DrawForParticle(UINT instanceCount_) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView()); // VBVを設定
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), instanceCount_, 0, 0);
}

//=============================================================================
// アニメーション適用
//=============================================================================

void Model::ApplyAnimation() {
	for (Joint& joint : skeleton_.joints) {
		//対象のJointのAnimationがあれば、値の適用を行う。
		if (auto it = animation_.GetNodeAnimations().find(joint.name); it != animation_.GetNodeAnimations().end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.scale = Animation::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
			joint.transform.rotate = Animation::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
			joint.transform.translate = Animation::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
		}
	}
}

//=============================================================================
// objファイルを読む関数
//=============================================================================

ModelData Model::LoadModelFile(const std::string& modelDirectoryPath, const std::string& filename) {

	Assimp::Importer importer;
	std::string filePath = "./Resources/" + modelDirectoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); //メッシュがない場合はエラー

	//Meshの解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); //法線がない場合は現在エラー
		assert(mesh->HasTextureCoords(0)); //UVがない場合は現在エラー

		//faceの解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); //三角形以外はエラー

			//vertexの解析
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertexData;
				vertexData.position = { position.x,position.y,position.z, 1.0f };
				vertexData.normal = { normal.x,normal.y,normal.z };
				vertexData.texcoord = { texcoord.x,texcoord.y };

				//aiProcess_MakeLeftHandedはz*=-1で、右手->左手に変換するので手動で対処する
				vertexData.position.x *= -1.0f;
				vertexData.normal.x *= -1.0f;
				modelData_.vertices.push_back(vertexData);
			}
		}
	}

	//materialの解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		unsigned int textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		textureCount;
		
		aiString textureFilePath;
		if(material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS){
			modelData_.material.textureFilePath = std::string("./Resources/images/") + textureFilePath.C_Str();
		} 
		
		if(modelData_.material.textureFilePath == "") { //テクスチャがない場合はデフォルトのテクスチャを設定
			modelData_.material.textureFilePath = "./Resources/images/uvChecker.png";
		}
	}

	//rootNodeの解析
	modelData_.rootNode = ReadNode(scene->mRootNode);

	return modelData_;
}

//=============================================================================
// mtlファイルを読む関数
//=============================================================================

ModelMaterialData Model::LoadMtlFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename) {

	ModelMaterialData materialData; //構築するMaterialData
	std::string line; //ファイルから読んだ1行を核のするもの
	std::ifstream file(resourceDirectoryPath + "/" + modelDirectoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFileName;
			s >> textureFileName;

			//連結してファイルパスにする
			materialData.textureFilePath = resourceDirectoryPath + "/" + textureFileName;
		}
	}

	return materialData;
}

//=============================================================================
// Nodeの解析
//=============================================================================

Node Model::ReadNode(aiNode* node) {
	Node result;
	aiVector3D scale,translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w }; //x軸を反転
	result.transform.translate = { -translate.x,translate.y,translate.z }; //x軸を反転
	result.localMatrix = MatrixMath::MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	//aiMatrix4x4 aiLocalMatrix = node->mTransformation; //nodeのlocalMatrix
	//aiLocalMatrix.Transpose(); //列ベクトルを行ベクトルの転置
	//for (int row = 0; row < 4; ++row) {
	//	for (int column = 0; column < 4; ++column) {
	//		result.localMatrix.m[row][column] = aiLocalMatrix[row][column];
	//	}
	//}

	result.name = node->mName.C_Str(); //名前の格納
	result.children.resize(node->mNumChildren); //子ノードの数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]); //再帰的に子ノードを読む
	}
	return result;
}

//

Skeleton Model::CreateSkeleton(const Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);
	
	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MatrixMath::MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint); //SkeletonのJoint列に追加
	for (const Node& child : node.children) {
		//子Jointを作成して、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	//自身のIndexを返す
	return joint.index;
}