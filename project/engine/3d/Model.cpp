#include "Model.h"
#include "base/DirectXCommon.h"
#include "base/TextureManager.h"
#include "3d/Material.h"
#include "math/MatrixMath.h"
#include "Utility/ResourceBarrier.h"

#include <fstream>
#include <sstream>

//=============================================================================
// 初期化
//=============================================================================

void Model::Initialize(ModelCommon* ModelCommon, std::unique_ptr<ModelData> modelData) {
	modelCommon_ = ModelCommon;

	//objファイル読み込み
	modelData_ = std::move(modelData);
	
	//Skeleton作成
	if (modelData_->haveBone) {
		skeleton_ = std::make_unique<Skeleton>();
		skeleton_->Create(modelData_->rootNode);
		//SkinCluster作成
		skinCluster_.Create(modelCommon_->GetDirectXCommon()->GetDevice(), modelCommon_->GetSrvManager(), skeleton_.get(), modelData_.get());
		haveSkeleton_ = true;
	} else {
		skeleton_ = nullptr;
		haveSkeleton_ = false;
	}

	//inputVertexResource
	modelData_->mesh.InitializeInputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice());
	//indexResource
	modelData_->mesh.InitializeIndexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice());
	//bufferViewの初期化
	modelData_->mesh.InitBufferViews();

	if (modelData_->haveBone) {
		//outputVertexResource
		modelData_->mesh.InitializeOutputVertexResourceModel(
			modelCommon_->GetDirectXCommon()->GetDevice(),
			modelCommon_->GetDirectXCommon()->GetCommandList());

		//SRVの設定
		inputIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
			modelData_->skinningInfoData.VertexCount,
			sizeof(VertexData),
			modelData_->mesh.GetInputVertexResource(),
			inputIndex_);

		//UAVの設定
		uavIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
			modelData_->skinningInfoData.VertexCount,
			sizeof(VertexData),
			modelData_->mesh.GetOutputVertexResource(),
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

	auto vbvs = modelData_->mesh.GetVBVs();
	auto ibvs = modelData_->mesh.GetIBVs();

	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//vbv設定
	//commandList->IASetVertexBuffers(0, UINT(vbvs.size()), vbvs.data());

	for (size_t i = 0; i < modelData_->mesh.GetSubMeshes().size(); ++i) {


		//materialCBufferの場所を指定
		commandList->SetGraphicsRootConstantBufferView(
			graphicPso->GetGraphicBindResourceIndex("gMaterial"),
			modelData_->mesh.GetSubMeshes()[i].material_.GetMaterialResource()->GetGPUVirtualAddress());
		//textureSRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			graphicPso->GetGraphicBindResourceIndex("gTexture"),
			TextureManager::GetInstance()->GetSrvIndex(modelData_->mesh.GetSubMeshes()[i].material_.GetTextureFilePath()));
		//envMapSRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			graphicPso->GetGraphicBindResourceIndex("gEnvMapTexture"),
			TextureManager::GetInstance()->GetSrvIndex(modelData_->mesh.GetSubMeshes()[i].material_.GetEnvMapFilePath()));
		//IBVの設定
		commandList->IASetVertexBuffers(0, 1, &vbvs[i]);
		modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&ibvs[i]);

		//DrawCall
		commandList->DrawIndexedInstanced(modelData_->mesh.GetSubMeshes()[i].indexCount, 1, modelData_->mesh.GetSubMeshes()[i].indexStart, 0, 0);
	}
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
		modelData_->mesh.GetOutputVertexResource());

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
	commandList->Dispatch(UINT(modelData_->skinningInfoData.VertexCount + 1023) / 1024, 1, 1);

	//UNORDERED_ACCESS >> VERTEX_AND_CONSTANT_BUFFER
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		modelData_->mesh.GetOutputVertexResource());
}


//=============================================================================
// パーティクル用描画処理
//=============================================================================

void Model::DrawForParticle(PSO* graphicPso, UINT instanceCount_) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	auto vbvs = modelData_->mesh.CreateSubMeshVBVs();
	auto ibvs = modelData_->mesh.CreateSubMeshIBVs();

	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (size_t i = 0; i < modelData_->mesh.GetSubMeshes().size(); ++i) {
		// VBVを設定	
		commandList->IASetVertexBuffers(0, 1, &vbvs[i]);

		//materialCBufferの場所を指定
		commandList->SetGraphicsRootConstantBufferView(
			graphicPso->GetGraphicBindResourceIndex("gMaterial"),
			modelData_->mesh.GetSubMeshes()[i].material_.GetMaterialResource()->GetGPUVirtualAddress());
		//textureSRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			graphicPso->GetGraphicBindResourceIndex("gTexture"),
			TextureManager::GetInstance()->GetSrvIndex(modelData_->mesh.GetSubMeshes()[i].material_.GetTextureFilePath()));
		//envMapSRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			graphicPso->GetGraphicBindResourceIndex("gEnvMapTexture"),
			TextureManager::GetInstance()->GetSrvIndex(modelData_->mesh.GetSubMeshes()[i].material_.GetEnvMapFilePath()));
		//IBVの設定
		modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&ibvs[i]);

		//DrawCall
		commandList->DrawIndexedInstanced(modelData_->mesh.GetSubMeshes()[i].indexCount, instanceCount_, 0, 0, 0);
	}
}

void Model::DrawForGPUParticle(PSO* graphicPso,UINT instanceCount) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	auto vbvs = modelData_->mesh.CreateSubMeshVBVs();
	auto ibvs = modelData_->mesh.CreateSubMeshIBVs();

	// 形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (size_t i = 0; i < modelData_->mesh.GetSubMeshes().size(); ++i) {
		// VBVを設定
		commandList->IASetVertexBuffers(0, 1, &vbvs[i]);

		//materialCBufferの場所を指定
		commandList->SetGraphicsRootConstantBufferView(
			graphicPso->GetGraphicBindResourceIndex("gMaterial"),
			modelData_->mesh.GetSubMeshes()[i].material_.GetMaterialResource()->GetGPUVirtualAddress());
		//textureSRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			graphicPso->GetGraphicBindResourceIndex("gTexture"),
			TextureManager::GetInstance()->GetSrvIndex(modelData_->mesh.GetSubMeshes()[i].material_.GetTextureFilePath()));
		//envMapSRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			graphicPso->GetGraphicBindResourceIndex("gEnvMapTexture"),
			TextureManager::GetInstance()->GetSrvIndex(modelData_->mesh.GetSubMeshes()[i].material_.GetEnvMapFilePath()));
		//IBVの設定
		modelCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&ibvs[i]);

		//DrawCall
		commandList->DrawIndexedInstanced(modelData_->mesh.GetSubMeshes()[i].indexCount, instanceCount, 0, 0, 0);
	}
}

std::unique_ptr<Model> Model::Clone() const {
	auto cloneModel = std::make_unique<Model>();

	//モデルデータのクローン
	cloneModel->modelData_ = modelData_->Clone();
	//スケルトンのクローン
	if (skeleton_) {
		cloneModel->skeleton_ = skeleton_->Clone();
	} else {
		cloneModel->skeleton_ = nullptr;
	}
	//スキンクラスターのクローン
	cloneModel->skinCluster_ = skinCluster_;
	cloneModel->modelCommon_ = modelCommon_;
	cloneModel->haveSkeleton_ = haveSkeleton_;
	cloneModel->translate_ = translate_;
	cloneModel->rotate_ = rotate_;
	cloneModel->scale_ = scale_;
	cloneModel->localMatrix_ = localMatrix_;
	cloneModel->inputIndex_ = inputIndex_;
	cloneModel->uavIndex_ = uavIndex_;
	
	return cloneModel;
}

const std::string& Model::GetTextureFilePath(const uint32_t& materialNum) const {
	
	if (materialNum < modelData_->mesh.GetSubMeshes().size()) {
		return modelData_->mesh.GetSubMeshes()[materialNum].material_.GetTextureFilePath();
	}
	//0番のテクスチャパスを返す
	return modelData_->mesh.GetSubMeshes()[0].material_.GetTextureFilePath();
}
