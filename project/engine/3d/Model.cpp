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

void Model::Initialize(ModelCommon* ModelCommon, ModelData& modelData, const std::string& modelDirectoryPath, const std::string& filePath) {
	modelCommon_ = ModelCommon;

	//objファイル読み込み
	modelData_ = modelData;
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
// Skeletonの作成
//=============================================================================

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

//=============================================================================
// NodeからJointを作成
//=============================================================================

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