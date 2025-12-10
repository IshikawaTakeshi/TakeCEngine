#include "Object3d.h"
#include "Object3dCommon.h"
#include "MatrixMath.h"
#include "DirectXCommon.h"
#include "Mesh/Mesh.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include "ImGuiManager.h"

#include <fstream>
#include <sstream>
#include <cassert>

Object3d::~Object3d() {
	wvpResource_.Reset();
	model_ = nullptr;
}

//=============================================================================
// 初期化
//=============================================================================

void Object3d::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	object3dCommon_ = object3dCommon;

	//モデルの設定
	modelFilePath_ = filePath;
	model_ = ModelManager::GetInstance().CopyModel(modelFilePath_);
	model_->GetMesh()->GetMaterial()->SetEnableLighting(true);

	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix));
	wvpResource_->SetName(L"Object3d::wvpResource_");
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//単位行列を書き込んでおく
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//カメラのセット
	camera_ = object3dCommon_->GetDefaultCamera();

	//アニメーション初期化
	animation_ = std::make_unique<Animation>();
	animation_->duration = 0.0f;
	animationTime_ = 0.0f;
}

//=============================================================================
// 更新処理
//=============================================================================

void Object3d::Update() {


	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//親子付け処理
	if (parentWorldMatrix_) {
		worldMatrix_ = MatrixMath::Multiply(worldMatrix_, *parentWorldMatrix_);
	}

	//wvpの更新
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix =
			CameraManager::GetInstance().GetActiveCamera()->GetViewProjectionMatrix();
		WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		WVPMatrix_ = worldMatrix_;
	}

	worldPosition_ = MatrixMath::Transform({ 0.0f,0.0f,0.0f }, worldMatrix_);
	
	//Skeletonがある場合は更新
	if (model_->GetSkeleton()) {
		WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
		TransformMatrixData_->World = worldMatrix_;
		TransformMatrixData_->WVP = WVPMatrix_;
		TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
		AnimationUpdate();
	} 
	else { //Skeletonがない場合
		if (animation_->duration != 0.0f) {
			WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
			TransformMatrixData_->World = model_->GetLocalMatrix() * worldMatrix_;
			TransformMatrixData_->WVP = model_->GetLocalMatrix() * WVPMatrix_;
			TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
			AnimationUpdate();
		} else {
			WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
			TransformMatrixData_->World = worldMatrix_;
			TransformMatrixData_->WVP = model_->GetModelData()->rootNode.localMatrix * WVPMatrix_;
			TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
			AnimationUpdate();
		}
	}

	//materialの更新
	if (model_->GetMesh() && model_->GetMesh()->GetMaterial()) {
		model_->GetMesh()->GetMaterial()->Update();
	}
}

void Object3d::AnimationUpdate() {

	//アニメーションの時間を進める
	if (isAnimation_ == true) {

		animationTime_ += 1.0f / 60.0f;
	}

	//スケルトン・スキンクラスターの更新
	model_->Update(animation_.get(), animationTime_);

	//最後まで行ったら最初からリピート再生する
	animationTime_ = std::fmod(animationTime_, animation_->duration);
}

//=============================================================================
// ImGuiの更新
//=============================================================================

void Object3d::UpdateImGui([[maybe_unused]]const std::string& name) {

#ifdef _DEBUG
	//ImGuiの更新
	std::string windowName = "Object3d" + name;
	ImGui::Begin("Object3d");
	if (ImGui::TreeNode(windowName.c_str())) {
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
		ImGui::Checkbox("isAnimation", &isAnimation_);
		
		model_->UpdateImGui();
		ImGui::TreePop();
	}
	ImGui::End();
#endif // _DEBUG
}

//=============================================================================
// 描画
//=============================================================================

void Object3d::Draw() {

	ID3D12GraphicsCommandList* commandList = object3dCommon_->GetDirectXCommon()->GetCommandList();

	//wvp用のCBufferの場所を指定
	commandList->SetGraphicsRootConstantBufferView(0, wvpResource_->GetGPUVirtualAddress());

	if(model_ != nullptr) {
		model_->Draw(object3dCommon_->GetPSO());
	}
}

void Object3d::Dispatch() {
	if (model_ != nullptr) {
		if (model_->GetSkeleton()) {
			model_->Dispatch(object3dCommon_->GetPSO());
		}
	}
}

//=============================================================================
// モデルの設定
//=============================================================================

const Vector3& Object3d::GetCenterPosition() const {

	return worldPosition_;
}

void Object3d::SetWorldMatrix(const Matrix4x4& worldMatrix) {

	worldMatrix_ = worldMatrix;
}

void Object3d::SetAnimation(Animation* animation) {
	if (animation) {
		animation_ = std::make_unique<Animation>(*animation);
	}
	animationTime_ = 0.0f;
}