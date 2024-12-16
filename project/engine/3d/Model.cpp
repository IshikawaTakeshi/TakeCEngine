#include "Model.h"
#include "DirectXCommon.h"
#include "Material.h"
#include "MatrixMath.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelCommon.h"
#include "Mesh/Mesh.h"

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
	animation_ = Animator::LoadAnimationFile(modelDirectoryPath, filePath);
	//Skeleton作成
	skeleton_ = CreateSkeleton(modelData_.rootNode);
	//SkinCluster作成
	skinCluster_.Create(modelCommon_->GetDirectXCommon()->GetDevice(), modelCommon_->GetSrvManager(), skeleton_, modelData_);

	//メッシュ初期化
	mesh_ = std::make_unique<Mesh>();
	mesh_->InitializeMesh(modelCommon_->GetDirectXCommon(), modelData_.material.textureFilePath);

	//VertexResource
	mesh_->InitializeVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	mesh_->AddVertexBufferView(skinCluster_.influenceBufferView);
	//indexResource
	mesh_->InitializeIndexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//MaterialResource
	mesh_->GetMaterial()->InitializeMaterialResource(modelCommon_->GetDirectXCommon()->GetDevice());
}

//=============================================================================
// 更新処理
//=============================================================================

void Model::Update() {

	//アニメーションがない場合は何もしない
	if (animation_.duration == 0.0f) {
		return;
	}

	//60fpsで進める
	//MEMO: 計測した時間を使って可変フレーム対応するのが望ましい
	animationTime += 1.0f / 60.0f;

	//アニメーションの更新とボーンへの適用
	ApplyAnimation();

	//スケルトンの更新
	UpdateSkeleton();

	//SkinClusterの更新
	skinCluster_.Update(skeleton_);

	//最後まで行ったら最初からリピート再生する
	animationTime = std::fmod(animationTime, animation_.duration);
}

//=============================================================================
// スケルトンの更新
//=============================================================================

void Model::UpdateSkeleton() {
	//全てのJointを更新
	int i = 0;
	for (Joint& joint : skeleton_.joints) {

		//ローカル行列を更新
		joint.localMatrix = MatrixMath::MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) { //親がいる場合親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton_.joints[*joint.parent].skeletonSpaceMatrix;
		} else { //親がいない場合は自身の行列をそのまま使う
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}

		i++;
	}
}

//=============================================================================
// 描画処理
//=============================================================================

void Model::Draw() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));
	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
}

void Model::DrawForASkinningModel() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	D3D12_VERTEX_BUFFER_VIEW vbv[] = { mesh_->GetVertexBufferView(0),mesh_->GetVertexBufferView(1) };

	// VBVを設定
	commandList->IASetVertexBuffers(0, 2, vbv);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));

	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(7, skinCluster_.useSrvIndex);
	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
}

void Model::DrawSkeleton() {


}

//=============================================================================
// パーティクル用描画処理
//=============================================================================

void Model::DrawForParticle(UINT instanceCount_) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0); 
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), instanceCount_, 0, 0);
}

//void Model::DrawSkeleton() {
//
//
//}

//=============================================================================
// アニメーション適用
//=============================================================================

void Model::ApplyAnimation() {
	int i = 0;
	for (Joint& joint : skeleton_.joints) {
		//対象のJointのAnimationがあれば、値の適用を行う。
		if (auto it = animation_.nodeAnimations.find(joint.name); it != animation_.nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.scale = Animator::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
			joint.transform.rotate = Animator::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
			joint.transform.translate = Animator::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
			//joint.localMatrix = MatrixMath::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		
		}
		i++;
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
		modelData_.vertices.resize(mesh->mNumVertices);

		//vertexの解析
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

			modelData_.vertices[vertexIndex].position = { -position.x,position.y,position.z, 1.0f };
			modelData_.vertices[vertexIndex].normal = { -normal.x,normal.y,normal.z };
			modelData_.vertices[vertexIndex].texcoord = { texcoord.x,texcoord.y };

		}
		//faceの解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); //三角形以外はエラー

			//Indices解析
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t indicesIndex = face.mIndices[element];
				modelData_.indices.push_back(indicesIndex);
			}		
		}

		//boneの解析
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointwName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData_.skinClusterData[jointwName];

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

	//materialの解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		unsigned int textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		textureCount;

		aiString textureFilePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
			modelData_.material.textureFilePath = std::string("./Resources/images/") + textureFilePath.C_Str();
		}

		if (modelData_.material.textureFilePath == "") { //テクスチャがない場合はデフォルトのテクスチャを設定
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
	aiVector3D scale, translate;
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

	for (Joint& joint : skeleton_.joints) {

		//ローカル行列を更新
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