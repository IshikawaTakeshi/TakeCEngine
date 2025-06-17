#include "Model.h"
#include "base/DirectXCommon.h"
#include "base/TextureManager.h"
#include "3d/Material.h"

#include "3d/Mesh/Mesh.h"
#include "math/MatrixMath.h"
#include "Utility/ResourceBarrier.h"

#include <fstream>
#include <sstream>

Model::Model() {}

Model::~Model() {

	mesh_.reset();
}

//=============================================================================
// 初期化
//=============================================================================

void Model::Initialize(ModelCommon* ModelCommon, ModelData* modelData) {
	modelCommon_ = ModelCommon;

	//objファイル読み込み
	modelData_ = modelData;

	//Skeleton作成
	if (modelData_->haveBone) {
		skeleton_ = std::make_unique<Skeleton>();
		skeleton_->Create(modelData_->rootNode);
		//SkinCluster作成
		skinCluster_.Create(modelCommon_->GetDirectXCommon()->GetDevice(), modelCommon_->GetSrvManager(), skeleton_.get(), modelData_);
		haveSkeleton_ = true;
	} else {
		skeleton_ = nullptr;
		haveSkeleton_ = false;
	}

	//メッシュ初期化
	mesh_ = std::make_unique<Mesh>();
	mesh_->InitializeMesh(
		modelCommon_->GetDirectXCommon(),
		modelData_->materials[].textureFilePath,
		modelData_->materials[].envMapFilePath);

	//inputVertexResource
	mesh_->InitializeInputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//indexResource
	mesh_->InitializeIndexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);

	if (modelData_->haveBone) {
		//outputVertexResource
		mesh_->InitializeOutputVertexResourceModel(
			modelCommon_->GetDirectXCommon()->GetDevice(),
			modelData_,
			modelCommon_->GetDirectXCommon()->GetCommandList());

		//SRVの設定
		inputIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
			modelData_->meshes[].skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetInputVertexResource(),
			inputIndex_);

		//UAVの設定
		uavIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
			modelData_->meshes[].skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetOutputVertexResource(),
			uavIndex_);
	} 
}

//=============================================================================
// 更新処理
//=============================================================================

void Model::Update(Animation* animation,float animationTime) {

	//アニメーションがない場合は何もしない
	if (animation->duration == 0.0f) {
		return;
	}

	if (haveSkeleton_) {
		//アニメーションの更新とボーンへの適用
		skeleton_->ApplyAnimation(animation, animationTime);

		//スケルトンの更新
		skeleton_->Update();

		//SkinClusterの更新
		skinCluster_.Update(skeleton_.get());
	} else {

		//rootNodeのAnimationを取得
		NodeAnimation& rootNodeAnimation = animation->nodeAnimations[modelData_->rootNode.name];
		translate_ = Animator::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
		rotate_ = Animator::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
		scale_ = Animator::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
		localMatrix_ = MatrixMath::MakeAffineMatrix(scale_, rotate_, translate_);
	}
}

//=============================================================================
// 描画処理
//=============================================================================

void Model::Draw(PSO* graphicPso) {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(
		graphicPso->GetGraphicBindResourceIndex("gMaterial"),
		mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//textureSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		graphicPso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance()->GetSrvIndex(modelData_->materials[].textureFilePath));
	//envMapSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		graphicPso->GetGraphicBindResourceIndex("gEnvMapTexture"),
		TextureManager::GetInstance()->GetSrvIndex(modelData_->materials[].envMapFilePath));
	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_->meshes[].indices.size()), 1, 0, 0, 0);
}

void Model::DrawSkyBox(PSO* graphicPso) {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(
		graphicPso->GetGraphicBindResourceIndex("gMaterial"),
		mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//TextureSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		graphicPso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance()->GetSrvIndex(modelData_->meshes[].material.textureFilePath));

	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_->meshes[].indices.size()), 1, 0, 0, 0);
}

//=============================================================================
// スキンメッシュの計算処理
//=============================================================================

void Model::DisPatch(PSO* skinningPso) {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VERTEX_AND_CONSTANT_BUFFER >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		mesh_->GetOutputVertexResource());

	//skinninginfo
	commandList->SetComputeRootConstantBufferView(
		skinningPso->GetComputeBindResourceIndex("gSkinningInfo"),
		skinCluster_.skinningInfoResource->GetGPUVirtualAddress());

	SrvManager* pSrvManager = modelCommon_->GetSrvManager();
	//palette
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gPalette"), skinCluster_.paletteIndex);
	//inputVertex
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gInputVertices"), inputIndex_);
	//influence
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gInfluences"), skinCluster_.influenceIndex);
	//outputVertex
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gOutputVertices"), uavIndex_);

	//DisPatch
	commandList->Dispatch(UINT(modelData_->meshes[].skinningInfoData.numVertices + 1023) / 1024, 1, 1);

	//UNORDERED_ACCESS >> VERTEX_AND_CONSTANT_BUFFER
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		mesh_->GetOutputVertexResource());
}


//=============================================================================
// パーティクル用描画処理
//=============================================================================

void Model::DrawForParticle(PSO* graphicPso, UINT instanceCount_) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//gMaterial
	commandList->SetGraphicsRootConstantBufferView(
		graphicPso->GetGraphicBindResourceIndex("gMaterial"),
		mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//gTexture
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		graphicPso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance()->GetSrvIndex(modelData_->materials[].textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_->meshes[].vertices.size()), instanceCount_, 0, 0);

}

void Model::DrawForGPUParticle(PSO* graphicPso,UINT instanceCount) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//gMaterial
	commandList->SetGraphicsRootConstantBufferView(
		graphicPso->GetGraphicBindResourceIndex("gMaterial"),
		mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//gTexture
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		graphicPso->GetGraphicBindResourceIndex("gTexture"),
		TextureManager::GetInstance()->GetSrvIndex(modelData_->materials[].textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_->meshes[].vertices.size()), instanceCount, 0, 0);
}

const std::string& Model::GetTextureFilePath(const uint32_t& materialNum) const {
	
	if (materialNum < modelData_->materials.size()) {
		return modelData_->materials[materialNum].textureFilePath;
	}
	//0番のテクスチャパスを返す
	return modelData_->materials[0].textureFilePath; 
}
