#include "Object3d.h"
#include "Object3dCommon.h"
#include "MatrixMath.h"
#include "DirectXCommon.h"
#include "Mesh.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Model.h"
#include "Camera.h"
#include "CameraManager.h"
#include "ImGuiManager.h"
#include <fstream>
#include <sstream>
#include <cassert>

Object3d::~Object3d() {
	wvpResource_.Reset();
	directionalLightResource_.Reset();
}

void Object3d::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	object3dCommon_ = object3dCommon;
	SetModel(filePath);
	model_->GetMesh()->GetMaterial()->SetEnableLighting(true);

	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix));
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
	//単位行列を書き込んでおく
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(DirectionalLightData));
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 1.0f;
	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = object3dCommon_->GetDefaultCamera();
}

void Object3d::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix =
			CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
		WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		WVPMatrix_ = worldMatrix_;
	}

	WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
	
	TransformMatrixData_->WVP = WVPMatrix_;
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
}

#ifdef _DEBUG
void Object3d::UpdateImGui(int id) {

	//ImGuiの更新
	std::string windowName = "Object3d" + std::to_string(id);
	ImGui::Begin("Object3d");
	if (ImGui::TreeNode(windowName.c_str())) {
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
		model_->GetMesh()->GetMaterial()->UpdateMaterialImGui();
		ImGui::Text("Lighting");
		ImGui::ColorEdit4("Color", &directionalLightData_->color_.x);
		ImGui::SliderFloat3("Direction", &directionalLightData_->direction_.x, -1.0f, 1.0f);
		ImGui::DragFloat("Intensity", &directionalLightData_->intensity_, 0.01f);
		object3dCommon_->UpdateImGui();
		ImGui::TreePop();
	}
	ImGui::End();
}
#endif // _DEBUG

void Object3d::Draw() {

	//wvp用のCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	//Lighting用のCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//カメラ情報のCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(
		4,CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

	if(model_ != nullptr) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filePath) {
		model_ = ModelManager::GetInstance()->FindModel(filePath);
}