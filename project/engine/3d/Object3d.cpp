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
}

//=============================================================================
// 初期化
//=============================================================================

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

//=============================================================================
// 更新処理
//=============================================================================

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

	
	//Animationがある場合は更新
	if (model_->GetAnimation().GetDuration() != 0.0f) {
		WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_* model_->GetLocalMatrix());
		model_->Update();
	TransformMatrixData_->World = model_->GetLocalMatrix() * worldMatrix_;
	TransformMatrixData_->WVP = model_->GetLocalMatrix() * WVPMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
	} 
	else { //Animationがない場合
		WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
		TransformMatrixData_->World = model_->GetModelData().rootNode.localMatrix * worldMatrix_;
		TransformMatrixData_->WVP = model_->GetModelData().rootNode.localMatrix * WVPMatrix_;
		TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
	}
}

//=============================================================================
// ImGuiの更新
//=============================================================================

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
		object3dCommon_->GetPSO()->UpdateImGui();
		ImGui::TreePop();
	}
	ImGui::End();
}
#endif // _DEBUG

//=============================================================================
// 描画
//=============================================================================

void Object3d::Draw() {

	//wvp用のCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());


	if(model_ != nullptr) {
		model_->Draw();
	}
}

//=============================================================================
// モデルの設定
//=============================================================================

void Object3d::SetModel(const std::string& filePath) {
		model_ = ModelManager::GetInstance()->FindModel(filePath);
}