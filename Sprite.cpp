#include "Sprite.h"
#include "DirectXCommon.h"
#include "MyMath/MatrixMath.h"
#include "Texture.h"

#pragma region imgui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma endregion


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
	transformationMatrixResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));

	//TransformationMatrix用
	transformationMatrixResource_->
	Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

	//単位行列を書き込んでおく
	*wvpData_ = MatrixMath::MakeIdentity4x4();

	//======================= MatrialResource ===========================//

	InitializeMaterialData(dxCommon);

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
	*wvpData_ = worldViewProjectionMatrix_;
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
	*wvpData_ = worldViewProjectionMatrix_;


	//ImGuiの更新
	ImGui::Begin("Sprite");
	ImGui::DragFloat3("SpriteTranslate", &transform_.translate.x, 1.0f);
	ImGui::End();
}

#pragma endregion

#pragma region 解放処理
void Sprite::Finalize() {}

#pragma endregion

void Sprite::InitializeMaterialData(DirectXCommon* dxCommon) {

	//マテリアル用リソース作成
	materialResourceSprite_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Vector4));
	//materialにデータを書き込む
	materialDataSprite_ = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite_));
	//色を書き込む
	*materialDataSprite_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
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
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	dxCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);
}