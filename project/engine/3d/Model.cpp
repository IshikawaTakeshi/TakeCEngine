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

Model::~Model() {

	mesh_.reset();
}

//=============================================================================
// 初期化
//=============================================================================

void Model::Initialize(ModelCommon* ModelCommon, ModelData& modelData) {
	modelCommon_ = ModelCommon;

	//objファイル読み込み
	modelData_ = modelData;

	//Skeleton作成
	if (modelData_.haveBone) {
		skeleton_ = std::make_unique<Skeleton>();
		skeleton_->Create(modelData_.rootNode);
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
		modelData_.material.textureFilePath,
		modelData_.material.envMapFilePath);

	//inputVertexResource
	mesh_->InitializeInputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//mesh_->MapInputVertexResource(modelData_);
	//indexResource
	mesh_->InitializeIndexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);

	if (modelData_.haveBone) {
		//outputVertexResource
		mesh_->InitializeOutputVertexResourceModel(
			modelCommon_->GetDirectXCommon()->GetDevice(),
			modelData_,
			modelCommon_->GetDirectXCommon()->GetCommandList());

		//SRVの設定
		inputIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
			modelData_.skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetInputVertexResource(),
			inputIndex_);

		

		//UAVの設定
		uavIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
			modelData_.skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetOutputVertexResource(),
			uavIndex_);
	} 
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

	if (haveSkeleton_) {
		//アニメーションの更新とボーンへの適用
		skeleton_->ApplyAnimation(animation_, animationTime);

		//スケルトンの更新
		skeleton_->Update();

		//SkinClusterの更新
		skinCluster_.Update(skeleton_.get());
	} else {

		//rootNodeのAnimationを取得
		NodeAnimation& rootNodeAnimation = animation_.nodeAnimations[modelData_.rootNode.name];
		translate_ = Animator::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
		rotate_ = Animator::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
		scale_ = Animator::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
		localMatrix_ = MatrixMath::MakeAffineMatrix(scale_, rotate_, translate_);
	}
	//最後まで行ったら最初からリピート再生する
	animationTime = std::fmod(animationTime, animation_.duration);
}

//=============================================================================
// スケルトンの更新
//=============================================================================

//void Model::UpdateSkeleton() {
//
//}

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
	commandList->SetGraphicsRootConstantBufferView(2, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//textureSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(4, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));
	//envMapSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(6, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.envMapFilePath));
	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
}

void Model::DrawSkyBox() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(1, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//TextureSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));

	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
}

void Model::DrawForSkinningModel() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	//D3D12_VERTEX_BUFFER_VIEW vbv[] = { mesh_->GetVertexBufferView(0),mesh_->GetVertexBufferView(1) };

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(2, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//TextureSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(4, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath));
	//envMapSRV
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(6, TextureManager::GetInstance()->GetSrvIndex(modelData_.material.envMapFilePath));

	//IBVの設定
	modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());

	//DrawCall
	commandList->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
}

void Model::DisPatchForSkinningModel() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// UAV から Vertex Buffer へのバリア
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	//今回のバリアはTransition
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	uavBarrier.Transition.pResource = mesh_->GetOutputVertexResource();
	//遷移前(現在)のResourceState
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	//遷移後のResourceState
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &uavBarrier);

	//skinninginfo
	commandList->SetComputeRootConstantBufferView(0, skinCluster_.skinningInfoResource->GetGPUVirtualAddress());
	//parette
	modelCommon_->GetSrvManager()->SetComputeRootDescriptorTable(1, skinCluster_.paletteIndex);
	//inputVertex
	modelCommon_->GetSrvManager()->SetComputeRootDescriptorTable(2, inputIndex_);
	//influence
	modelCommon_->GetSrvManager()->SetComputeRootDescriptorTable(3, skinCluster_.influenceIndex);
	//outputVertex
	modelCommon_->GetSrvManager()->SetComputeRootDescriptorTable(4, uavIndex_);

	//DisPatch
	commandList->Dispatch(UINT(modelData_.skinningInfoData.numVertices + 1023) / 1024, 1, 1);


	//今回のバリアはTransition
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	uavBarrier.Transition.pResource = mesh_->GetOutputVertexResource();
	//遷移前(現在)のResourceState
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	//遷移後のResourceState
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &uavBarrier);
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

void Model::SetAnimation(Animation animation) {
	animation_ = animation;
	animationTime = 0.0f;
}

//void Model::SetAnimation(const std::string& modelDirectoryPath, const std::string& filename) {
//	animation_ = Animator::LoadAnimationFile(modelDirectoryPath, filename);
//	animationTime = 0.0f;
//}
