#include "Sprite.h"
#include "../Include/DirectXCommon.h"
#include "../MyMath/MatrixMath.h"
#include "../Texture/Texture.h"

#pragma region imgui
#ifdef _DEBUG

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // DEBUG

#pragma endregion


Sprite::~Sprite() {
	/*delete materialDataSprite_;
	delete vertexData_;
	delete wvpData_;*/
	materialResourceSprite_.Reset();
	wvpResource_.Reset();
	transformationMatrixResource_.Reset();
}

#pragma region 初期化処理

void Sprite::Initialize(DirectXCommon* dxCommon) {

	//======================= VertexResource ===========================//

	//VertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 6);
	//頂点バッファビューの作成
	vertexBufferView_.BufferLocation = wvpResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	//1枚目の三角形
	vertexData_[0].position = { 0.0f,90.0f,0.0f,1.0f }; //左下
	vertexData_[0].texcoord = { 0.0f,1.0f };
	vertexData_[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[1].position = { 0.0f,0.0f,0.0f,1.0f }; //左上
	vertexData_[1].texcoord = { 0.0f,0.0f };
	vertexData_[2].position = { 160.0f,90.0f,0.0f,1.0f }; //右下
	vertexData_[2].texcoord = { 1.0f,1.0f };
	//2枚目の三角形
	vertexData_[3].position = { 0.0f,0.0f,0.0f,1.0f }; //左上
	vertexData_[3].texcoord = { 0.0f,0.0f };
	vertexData_[4].position = { 160.0f,0.0f,0.0f,1.0f }; //右上
	vertexData_[4].texcoord = { 1.0f,0.0f };
	vertexData_[5].position = { 160.0f,90.0f,0.0f,1.0f }; //右下
	vertexData_[5].texcoord = { 1.0f,1.0f };

	//======================= transformationMatrix用のVertexResource ===========================//

	//スプライト用のTransformationMatrix用のVertexResource生成
	transformationMatrixResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	transformationMatrixResource_->
	Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));

	//単位行列を書き込んでおく
	transformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//======================= MaterialResource ===========================//

	InitializeMaterialData(dxCommon);

	//======================= DirectionalLightResource ===========================//

	InitializeDirectionalLightData(dxCommon);


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
	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;
}
#pragma endregion

#pragma region 更新処理
void Sprite::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの処理
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	transformMatrixData_->WVP = worldViewProjectionMatrix_;
	transformMatrixData_->World = worldMatrix_;

#ifdef _DEBUG
	//ImGuiの更新
	ImGui::Begin("Sprite");
	ImGui::DragFloat3("SpriteTranslate", &transform_.translate.x, 1.0f);

	ImGui::End();
#endif // DEBUG

}

#pragma endregion

#pragma region 解放処理
void Sprite::Finalize() {}

#pragma endregion

void Sprite::InitializeMaterialData(DirectXCommon* dxCommon) {

	//マテリアル用リソース作成
	materialResourceSprite_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	//materialにデータを書き込む
	materialDataSprite_ = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite_));
	//色を書き込む
	materialDataSprite_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite_->enableLighting = false;
}

void Sprite::InitializeDirectionalLightData(DirectXCommon* dxCommon) {

	//平行光源用Resourceの作成
	directionalLightResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));
	directionalLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//光源の色を書き込む
	directionalLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	directionalLightData_->direction_ = { 0.0f,-1.0f,0.0f };
	//光源の輝度書き込む
	directionalLightData_->intensity = 1.0f;
}

void Sprite::InitializeCommandList(DirectXCommon* dxCommon, Texture* texture) {
	//spriteの描画。
	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite_->GetGPUVirtualAddress());
	//TransformationMatrixCBufferの場所の設定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, texture->GetTextureSrvHandleGPU());
	//Lighting用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	dxCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);
}