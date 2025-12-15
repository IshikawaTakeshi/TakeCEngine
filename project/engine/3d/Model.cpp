#include "Model.h"

// STL
#include <fstream>
#include <sstream>

// Engine
#include "engine/base/DirectXCommon.h"
#include "engine/base/TextureManager.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/ModelManager.h"
#include "engine/3d/Material.h"
#include "engine/3d/Mesh/Mesh.h"
#include "engine/math/MatrixMath.h"
#include "engine/Utility/ResourceBarrier.h"

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
	mesh_->InitializeMesh(modelCommon_->GetDirectXCommon(),modelData_->materials);

	//inputVertexResource
	mesh_->InitializeInputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//indexResource
	mesh_->InitializeIndexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);

	if (modelData_->haveBone) {
		//outputVertexResource
		mesh_->InitializeOutputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(),modelData_);

		//SRVの設定
		inputIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
			modelData_->skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetInputVertexResource(),
			inputIndex_);

		//UAVの設定
		uavIndex_ = modelCommon_->GetSrvManager()->Allocate();
		modelCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
			modelData_->skinningInfoData.numVertices,
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
		//SkinClusterの更新
		if (haveSkeleton_) {
			skinCluster_.Update(skeleton_.get());
		}
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
		translate_ = TakeC::AnimationManager::CalculateValue(rootNodeAnimation.translate.keyframes, animationTime);
		rotate_ = TakeC::AnimationManager::CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime);
		scale_ = TakeC::AnimationManager::CalculateValue(rootNodeAnimation.scale.keyframes, animationTime);
		localMatrix_ = MatrixMath::MakeAffineMatrix(scale_, rotate_, translate_);
	}
}

//=============================================================================
// ImGui更新
//=============================================================================

void Model::UpdateImGui() {

	//モデル情報
	if(ImGui::CollapsingHeader("Model Info")) {

		ImGui::Text("Model Name: %s", modelData_->fileName.c_str());
		ImGui::Text("Num Vertices: %d", static_cast<int>(modelData_->vertices.size()));
		ImGui::Text("Num Indices: %d", static_cast<int>(modelData_->indices.size()));

		//サブメッシュの情報表示
		ImGui::Text("Num SubMeshes: %d", static_cast<int>(modelData_->subMeshes.size()));
		for (size_t i = 0; i < modelData_->subMeshes.size(); ++i) {
			const SubMesh& subMesh = modelData_->subMeshes[i];
			ImGui::Text("vertexCount:%d, vertexStart:%d", subMesh.vertexCount,subMesh.vertexStart);
			ImGui::Text(" SubMesh %d: IndexCount:%d, IndexStart:%d", static_cast<int>(i), subMesh.indexCount, subMesh.indexStart);
		}

		//マテリアル情報の表示
		mesh_->GetMaterial()->UpdateMaterialImGui();

		//モデルのリロード
		if(ImGui::Button("Reload Model")) {
			TakeC::ModelManager::GetInstance().RequestReload(modelData_->fileName);
		}

		//テクスチャのリロード
		if (ImGui::Button("Reload Texture")) {
			TakeC::TextureManager::GetInstance().LoadTexture(modelData_->material.textureFilePath, true);
		}
	}
}

//=============================================================================
// 描画処理
//=============================================================================

void Model::Draw(PSO* pso) {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBV/IBV の設定（共通）
	mesh_->SetVertexBuffers(commandList, 0);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetIndexBuffer(&mesh_->GetIndexBufferView());

	// サブメッシュごとに描画
	for (const SubMesh& sub : modelData_->subMeshes) {
		const ModelMaterialData& mat = modelData_->materials[sub.materialIndex];

		// マテリアル CBuffer
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"),
			mesh_->GetMaterials()[sub.materialIndex]->GetMaterialResource()->GetGPUVirtualAddress()
		);

		// Texture SRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"),
			TakeC::TextureManager::GetInstance().GetSrvIndex(mat.textureFilePath));

		// EnvMap SRV
		modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gEnvMapTexture"),
			TakeC::TextureManager::GetInstance().GetSrvIndex(mat.envMapFilePath));

		// DrawObject 呼び出し
		commandList->DrawIndexedInstanced(
			sub.indexCount,    // インデックス数
			1,                 // インスタンス数
			sub.indexStart,    // StartIndexLocation
			0,                 // BaseVertexLocation
			0                  // StartInstanceLocation
		);
	}
}

void Model::DrawShadow() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBV/IBV の設定（共通）
	mesh_->SetVertexBuffers(commandList, 0);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetIndexBuffer(&mesh_->GetIndexBufferView());

	// サブメッシュごとに描画
	for (const SubMesh& sub : modelData_->subMeshes) {
		// DrawObject 呼び出し
		commandList->DrawIndexedInstanced(
			sub.indexCount,    // インデックス数
			1,                 // インスタンス数
			sub.indexStart,    // StartIndexLocation
			0,                 // BaseVertexLocation
			0                  // StartInstanceLocation
		);
	}
}


//=============================================================================
// スキンメッシュの計算処理
//=============================================================================

void Model::Dispatch(PSO* skinningPso) {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VERTEX_AND_CONSTANT_BUFFER >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		mesh_->GetOutputVertexResource());

	//skinninginfo
	commandList->SetComputeRootConstantBufferView(
		skinningPso->GetComputeBindResourceIndex("gSkinningInfo"),
		skinCluster_.skinningInfoResource->GetGPUVirtualAddress());

	TakeC::SrvManager* pSrvManager = modelCommon_->GetSrvManager();
	//palette
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gPalette"), skinCluster_.paletteIndex);
	//inputVertex
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gInputVertices"), inputIndex_);
	//influence
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gInfluences"), skinCluster_.influenceIndex);
	//outputVertex
	pSrvManager->SetComputeRootDescriptorTable(skinningPso->GetComputeBindResourceIndex("gOutputVertices"), uavIndex_);

	//DisPatch
	commandList->Dispatch(UINT(modelData_->skinningInfoData.numVertices + 1023) / 1024, 1, 1);

	//UNORDERED_ACCESS >> VERTEX_AND_CONSTANT_BUFFER
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		mesh_->GetOutputVertexResource());
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
	commandList->SetGraphicsRootConstantBufferView(1, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TakeC::TextureManager::GetInstance().GetSrvIndex(modelData_->material.textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_->vertices.size()), instanceCount_, 0, 0);

}

void Model::DrawForGPUParticle(UINT instanceCount) {
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// VBVを設定
	mesh_->SetVertexBuffers(commandList, 0);
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(1, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	modelCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(2, TakeC::TextureManager::GetInstance().GetSrvIndex(modelData_->material.textureFilePath));
	//DrawCall
	commandList->DrawInstanced(UINT(modelData_->vertices.size()), instanceCount, 0, 0);
}

//=============================================================================
// モデルデータの再読み込み
//=============================================================================

void Model::Reload(ModelData* newModelData) {
	// 新しい ModelData に差し替え
	modelData_ = newModelData;

	//--------------------------------------------------------
	// Skeleton/SkinCluster
	//--------------------------------------------------------

	// 既存メッシュの再初期化
	if (modelData_->haveBone) {
		// Skeletonがなければ作成
		if(!skeleton_) {
			skeleton_ = std::make_unique<Skeleton>();
		}
		// Skeleton を新しい rootNode で構築し直す
		skeleton_->Create(modelData_->rootNode);

		// SkinCluster 作成
		skinCluster_.Create(
			modelCommon_->GetDirectXCommon()->GetDevice(),
			modelCommon_->GetSrvManager(),
			skeleton_.get(),
			modelData_);
		haveSkeleton_ = true;
	} else {
		//ボーンがないモデルに差し替えた場合はSkeletonを破棄
		skeleton_.reset();
		haveSkeleton_ = false;
	}

	//--------------------------------------------------------
	// メッシュ再初期化
	//--------------------------------------------------------

	// 既存メッシュの再初期化
	mesh_->InitializeMesh(
		modelCommon_->GetDirectXCommon(),modelData_->materials);

	//inputVertexResource
	mesh_->InitializeInputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);
	//indexResource
	mesh_->InitializeIndexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(), modelData_);

	//--------------------------------------------------------
	// スキニング用リソース再初期化
	//--------------------------------------------------------

	if(modelData_->haveBone) {
		//outputVertexResource
		mesh_->InitializeOutputVertexResourceModel(modelCommon_->GetDirectXCommon()->GetDevice(),modelData_);

		// 入力頂点用 SRV
		if (inputIndex_ == 0) {
			// 未割り当ての場合のみ新規割り当て
			inputIndex_ = modelCommon_->GetSrvManager()->Allocate();
		}

		//SRVの設定
		modelCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
			modelData_->skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetInputVertexResource(),
			inputIndex_);
		//UAVの設定
		// 出力頂点用 UAV
		if (uavIndex_ == 0) {
			// 未割り当ての場合のみ新規割り当て
			uavIndex_ = modelCommon_->GetSrvManager()->Allocate();
		}
		
		modelCommon_->GetSrvManager()->CreateUAVforStructuredBuffer(
			modelData_->skinningInfoData.numVertices,
			sizeof(VertexData),
			mesh_->GetOutputVertexResource(),
			uavIndex_);
	}

	//--------------------------------------------------------
	// ボーンを持たないモデルの場合、変換情報をリセット
	//--------------------------------------------------------

	if (!modelData_->haveBone) {
		translate_ = { 0.0f,0.0f,0.0f };
		rotate_ = { 0.0f,0.0f,0.0f,1.0f };
		scale_ = { 1.0f,1.0f,1.0f };
		localMatrix_ = modelData_->rootNode.localMatrix;
	}
}
