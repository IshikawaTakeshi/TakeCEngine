#include "Sphere.h"
#include "../MyMath/MatrixMath.h"
#include "../MyMath/MyMath.h"
#include "../Include/DirectXCommon.h"
#include "../Texture/Texture.h"
#include <numbers>

#pragma region imgui
#ifdef _DEBUG
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif 

Sphere::~Sphere() {
	materialResource_.Reset();;
	wvpResource_.Reset();
	vertexResource_.Reset();
}

void Sphere::Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView) {


	//======================= VertexResource ===========================//

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 緯度分割1つ分の角度

//VertexResource生成
	vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * kSubdivision * kSubdivision * 6);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; //現在の緯度(シータ)

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; //現在の経度(ファイ)
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			//1枚目の三角形
			//頂点データの入力。基準点a(左下)
			vertexData_[start].position.x = cos(lat) * cos(lon);
			vertexData_[start].position.y = sin(lat);
			vertexData_[start].position.z = cos(lat) * sin(lon);
			vertexData_[start].position.w = 1.0f;
			vertexData_[start].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData_[start].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));

		
			//基準点b1(左上)
			vertexData_[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData_[start + 1].position.y = sin(lat + kLatEvery);
			vertexData_[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData_[start + 1].position.w = 1.0f;
			vertexData_[start + 1].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData_[start + 1].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));


			//基準点c1(右下)
			vertexData_[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData_[start + 2].position.y = sin(lat);
			vertexData_[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData_[start + 2].position.w = 1.0f;
			vertexData_[start + 2].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData_[start + 2].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));


			//2枚目の三角形
			//基準点b2(左上)
			vertexData_[start + 3] = vertexData_[start + 1];


			//基準点d(右上)
			vertexData_[start + 4].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData_[start + 4].position.y = sin(lat + kLatEvery);
			vertexData_[start + 4].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData_[start + 4].position.w = 1.0f;
			vertexData_[start + 4].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData_[start + 4].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));


			
			//基準点c1(右下)
			vertexData_[start + 5] = vertexData_[start + 2];
			
		}
	}

	

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

void Sphere::Update() {


	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	*wvpData_ = worldViewProjectionMatrix_;


#ifdef _DEBUG
	ImGui::Begin("Window::Sphere");
	ImGui::DragFloat3("SphereScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("SphereRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("SphereTranslate", &transform_.translate.x, 0.01f);
	ImGui::End();
#endif // _DEBUG
}

void Sphere::InitializeMaterialData(DirectXCommon* dxCommon) {
	//マテリアル用リソース作成
	materialResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Vector4));
	//materialにデータを書き込む
	materialData_ = nullptr;
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//色を書き込む
	*materialData_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Sphere::InitializeCommandList(DirectXCommon* dxCommon, Texture* texture) {

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
	dxCommon->GetCommandList()->DrawInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0);
}

