#include "../Sphere/Sphere.h"
#include "../MyMath/MatrixMath.h"
#include "../MyMath/MyMath.h"
#include "../Include/DirectXCommon.h"
#include <numbers>

#pragma region imgui
#ifdef _DEBUG
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif 
#pragma endregion

Sphere::~Sphere() {
	
	directionalLightResource_.Reset();
	wvpResource_.Reset();
}

void Sphere::Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView, const std::string& textureFilePath) {

	//メッシュ初期化
	mesh_ = new Mesh();
	mesh_->InitializeMesh(1,dxCommon,textureFilePath);

	//======================= VertexResource ===========================//

	mesh_->InitializeVertexResourceSphere(dxCommon->GetDevice());

	//======================= transformationMatrix用のVertexResource ===========================//

	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));

	//単位行列を書き込んでおく
	transformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//======================= MaterialResource ===========================//

	mesh_->GetMaterial()->GetMaterialResource();

	//======================= DirectionalLightResource ===========================//

	InitializeDirectionalLightData(dxCommon);

	//======================= IndexResource ===========================//

	mesh_->InitializeIndexResourceSphere(dxCommon->GetDevice());

	//======================= Transform・各行列の初期化 ===========================//

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::Inverse(cameraView);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f
	);
}

void Sphere::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;


#ifdef _DEBUG
	ImGui::Begin("Window::Sphere");
	ImGui::DragFloat3("SphereScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("SphereRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("SphereTranslate", &transform_.translate.x, 0.01f);
	ImGui::Checkbox("useMonsterBall", &useMonsterBall);
	ImGui::ColorEdit4("LightColor", &directionalLightData_->color_.x);
	ImGui::SliderFloat3("LightDirection", &directionalLightData_->direction_.x, -1.0f, 1.0f);
	ImGui::SliderFloat("LightIntensity", &directionalLightData_->intensity_, 0.0f, 1.0f);
	MyMath::Normalize(directionalLightData_->direction_);
	ImGui::End();



#endif // _DEBUG
}

void Sphere::InitializeDirectionalLightData(DirectXCommon* dxCommon) {

	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLightData));
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity_ = 1.0f;
}

void Sphere::DrawCall(DirectXCommon* dxCommon) {

	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView()); // VBVを設定

	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	//dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
		0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, mesh_->GetMaterial()->GetTexture()->GetTextureSrvHandleGPU());
	//Lighting用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//IBVの設定
	dxCommon->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	// 描画！(DrawCall/ドローコール)
	dxCommon->GetCommandList()->DrawIndexedInstanced(mesh_->kSubdivision * mesh_->kSubdivision * 6, 1, 0, 0, 0);
}

