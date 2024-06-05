#include "Triangle.h"
#include "DirectXCommon.h"
#include "MyMath/MatrixMath.h"
#include "Texture.h"

#pragma region imgui
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG

#pragma endregion

Triangle::~Triangle() {
	//delete materialData_;
	materialResource_.Reset();
	//delete wvpData_;
	//delete vertexData_;
	wvpResource_.Reset();
	vertexResource_.Reset();
}

void Triangle::Initialize(DirectXCommon* dxCommon,Matrix4x4 cameraView) {

	//======================= VertexResource ===========================//
	
	//VertexResource生成
	vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 3);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	
	//左下
	vertexData_[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData_[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData_[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData_[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData_[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData_[2].texcoord = { 1.0f,1.0f };


	//======================= transformationMatrix用のVertexResource ===========================//
	
	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

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
	viewMatrix_ = MatrixMath::Inverse(cameraView);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f
	);
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	*wvpData_ = worldViewProjectionMatrix_;
}

void Triangle::Update(
#ifdef _DEBUG
	int id
#endif
) {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	*wvpData_ = worldViewProjectionMatrix_;

	bool botton = false;
	//ImGuiの更新
#ifdef _DEBUG
	std::string label = "Window::Triangle";
	label += "##" + std::to_string(id);
	ImGui::Begin(label.c_str());
	ImGui::ColorEdit4("triangleColor", &materialData_->x);
	ImGui::Selectable("TextureTable",botton);
	ImGui::DragFloat3("TriangleScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("TriangleRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("TriangleTranslate", &transform_.translate.x, 0.01f);
	ImGui::End();
#endif // DEBUG

}

void Triangle::InitializeMaterialData(DirectXCommon* dxCommon) {
	//マテリアル用リソース作成
	materialResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Vector4));
	//materialにデータを書き込む
	materialData_ = nullptr;
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//色を書き込む
	*materialData_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Triangle::InitializeCommandList(DirectXCommon* dxCommon,Texture* texture) {

	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, texture->GetTextureSrvHandleGPU());
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	dxCommon->GetCommandList()->DrawInstanced(3, 1, 0, 0);

}


