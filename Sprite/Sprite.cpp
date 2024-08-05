#include "Sprite.h"
#include "../Sprite/SpriteCommon.h"
#include "../Include/DirectXCommon.h"
#include "../Include/Mesh.h"
#include "../Include/Material.h"
#include "../MyMath/MatrixMath.h"
#include "../Texture/Texture.h"

#pragma region imgui
#ifdef _DEBUG

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif // DEBUG

#pragma endregion


Sprite::~Sprite() {

	directionalLightResource_.Reset();
	
}

#pragma region 初期化処理

void Sprite::Initialize(SpriteCommon* spriteCommon, const std::string& textureFilePath) {

	//SpriteCommonの設定
	spriteCommon_ = spriteCommon;

	//メッシュ初期化
	mesh_ = new Mesh();
	mesh_->InitializeMesh(1,spriteCommon_->GetDirectXCommon(),textureFilePath);
	//vertexResource初期化
	mesh_->InitializeVertexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice());
	//IndexResource初期化
	mesh_->InitializeIndexResourceSprite(spriteCommon->GetDirectXCommon()->GetDevice());
	//MaterialResource初期化
	mesh_->GetMaterial()->InitializeMaterialResource(spriteCommon->GetDirectXCommon()->GetDevice());

	//======================= transformationMatrix用のVertexResource ===========================//

	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(spriteCommon->GetDirectXCommon()->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->
		Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

	//単位行列を書き込んでおく
	wvpData_->WVP = MatrixMath::MakeIdentity4x4();

	
	//======================= DirectionalLightResource ===========================//

	InitializeDirectionalLightData(spriteCommon->GetDirectXCommon());

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
	viewMatrix_ = MatrixMath::MakeIdentity4x4();
	projectionMatrix_ = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;
}
#pragma endregion

#pragma region 更新処理
void Sprite::Update(int num) {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの処理
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	wvpData_->WVP = worldViewProjectionMatrix_;
	wvpData_->World = worldMatrix_;

#ifdef _DEBUG
	//ImGuiの更新
	std::string windowName = "Sprite" + std::to_string(num);
	ImGui::Begin(windowName.c_str());
	ImGui::DragFloat3("SpriteTranslate", &transform_.translate.x, 1.0f);
	ImGui::DragFloat3("SpriteRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("SpriteScale", &transform_.scale.x, 0.01f);
	mesh_->GetMaterial()->UpdateMaterialImGui();
	ImGui::End();
	
#endif // DEBUG

}

#pragma endregion

void Sprite::InitializeDirectionalLightData(DirectXCommon* dxCommon) {

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

#pragma region 描画処理
void Sprite::DrawCall(DirectXCommon* dxCommon) {
	//spriteの描画。
	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &mesh_->GetVertexBufferView()); // VBVを設定
	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
		0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//TransformationMatrixCBufferの場所の設定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, mesh_->GetMaterial()->GetTexture()->GetTextureSrvHandleGPU());
	//Lighting用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//IBVの設定
	dxCommon->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
#pragma endregion