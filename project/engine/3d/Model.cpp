#include "Model.h"
#include "DirectXCommon.h"
#include "Material.h"
#include "MatrixMath.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelCommon.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <sstream>
#include <cassert>



Model::~Model() {

	mesh_.reset();
}

void Model::Initialize(ModelCommon* ModelCommon, const std::string& filePath) {
	modelCommon_ = ModelCommon;

	//objファイル読み込み
	modelData_ = LoadObjFile("Resources", "obj_mtl_blend", filePath);

	//メッシュ初期化
	mesh_ = std::make_unique<Mesh>();
	mesh_->InitializeMesh(modelCommon_->GetDirectXCommon(), modelData_.material.textureFilePath);
	
	//VertexResource
	mesh_->InitializeVertexResourceObjModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//MaterialResource
	mesh_->GetMaterial()->InitializeMaterialResource(modelCommon_->GetDirectXCommon()->GetDevice());
}



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


ModelData Model::LoadObjFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename) {

	Assimp::Importer importer;
	std::string filePath = resourceDirectoryPath + "/" + modelDirectoryPath + "/" + filename;
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
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData_.material.textureFilePath = resourceDirectoryPath + "/" + textureFilePath.C_Str();
		}
	}

	return modelData_;
}

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
