#include "Triangle.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include "TextureManager.h"

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Triangle::~Triangle() {

	materialResource_.Reset();
	wvpResource_.Reset();
	vertexResource_.Reset();
}

void Triangle::Initialize(DirectXCommon* dxCommon,Matrix4x4 cameraView) {


	//======================= transformationMatrix用のVertexResource ===========================//
	
	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));

	//単位行列を書き込んでおく
	*TransformMatrixData_ = MatrixMath::MakeIdentity4x4();


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
	*TransformMatrixData_ = worldViewProjectionMatrix_;
}

void Triangle::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	*TransformMatrixData_ = worldViewProjectionMatrix_;
}

#ifdef _DEBUG
void Triangle::UpdateImGui(int id) {
	//ImGuiの更新

	bool botton = false;
	std::string label = "Window::Triangle";
	label += "##" + std::to_string(id);
	ImGui::Begin(label.c_str());
	ImGui::ColorEdit4("triangleColor", &materialData_->x);
	ImGui::Selectable("TextureTable", botton);
	ImGui::DragFloat3("TriangleScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("TriangleRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("TriangleTranslate", &transform_.translate.x, 0.01f);
	ImGui::End();
}
#endif // DEBUG

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

void Triangle::Draw(DirectXCommon* dxCommon) {

	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViews_); // VBVを設定
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	dxCommon->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}


