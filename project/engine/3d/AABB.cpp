#include "AABB.h"
//#include "MatrixMath.h"
//#include "Vector3Math.h"
//#include "Sphere.h"
//#include "ImGuiManager.h"
//#include "DirectXCommon.h"
//#include "Camera.h"
//#include "Mesh.h"
//#include "TextureManager.h"
//#include "TransformMatrix.h"
//#include "Transform.h"
//#include <algorithm>
//
//void AABB::Initialize(DirectXCommon* dxCommon, Vector3 min, Vector3 max) {
//
//	dxCommon_ = dxCommon;
//
//	mesh_ = new Mesh();
//	mesh_->InitializeMesh(dxCommon, "Resources/Texture/white.png");
//
//	mesh_->InitializeVertexResourceAABB(dxCommon->GetDevice());
//
//	mesh_->GetMaterial()->GetMaterialResource();
//
//	mesh_->InitializeIndexResourceAABB(dxCommon->GetDevice());
//
//	//スプライト用のTransformationMatrix用のVertexResource生成
//	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
//
//	//TransformationMatrix用
//	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
//
//	//単位行列を書き込んでおく
//	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();
//
//	//======================= Transform・各行列の初期化 ===========================//
//
//	//CPUで動かす用のTransform
//	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
//
//	//アフィン行列
//	worldMatrix_ = MatrixMath::MakeAffineMatrix(
//		transform_.scale,
//		transform_.rotate,
//		transform_.translate
//	);
//
//	min_ = min;
//	max_ = max;
//	color_ = 0xffffffff;
//}
//
//void AABB::Update() {
//	min_.x = (std::min)(min_.x, max_.x);
//	max_.x = (std::max)(min_.x, max_.x);
//	min_.y = (std::min)(min_.y, max_.y);
//	max_.y = (std::max)(min_.y, max_.y);
//	min_.z = (std::min)(min_.z, max_.z);
//	max_.z = (std::max)(min_.z, max_.z);
//
//	worldMatrix_ = MatrixMath::MakeAffineMatrix(
//		transform_.scale,
//		transform_.rotate,
//		transform_.translate
//	);
//
//	WVPMatrix_ = worldMatrix_ * camera_->GetViewProjectionMatrix();
//
//	TransformMatrixData_->World = worldMatrix_;
//	TransformMatrixData_->WVP = WVPMatrix_;
//}
//
//void AABB::UpdateImGui(std::string name) {
//	ImGui::Text(name.c_str());
//	ImGui::DragFloat3("aabb.min", &min_.x, 0.01f);
//	ImGui::DragFloat3("aabb.max", &max_.x, 0.01f);
//}
//
//void AABB::Draw(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
//	//spriteの描画。
//	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView()); // VBVを設定
//	//materialCBufferの場所を指定
//	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
//		0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
//	//TransformationMatrixCBufferの場所の設定
//	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
//	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath_));
//	//IBVの設定
//	dxCommon_->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
//	// 描画！(DrawCall/ドローコール)
//	dxCommon_->GetCommandList()->DrawIndexedInstanced(8, 1, 0, 0, 0);
//}
//
//bool AABB::IsCollision(const AABB& aabb2) {
//	if ((min_.x <= aabb2.max_.x && max_.x >= aabb2.min_.x) &&
//		(min_.y <= aabb2.max_.y && max_.y >= aabb2.min_.y) &&
//		(min_.z <= aabb2.max_.z && max_.z >= aabb2.min_.z)) {
//
//		return true;
//	}
//	return false;
//}