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
	//indexResource_.Reset();
	//directionalLightResource_.Reset();
	//materialResource_.Reset();
	//wvpResource_.Reset();
	//vertexResource_.Reset();
}

void Sphere::Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView) {


	//======================= VertexResource ===========================//

	InitializeVertexData(dxCommon);

	//======================= transformationMatrix用のVertexResource ===========================//

	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));

	//単位行列を書き込んでおく
	transformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//======================= MaterialResource ===========================//

	InitializeMaterialData(dxCommon);

	//======================= DirectionalLightResource ===========================//

	InitializeDirectionalLightData(dxCommon);

	//======================= IndexResource ===========================//

	InitializeIndexBufferView(dxCommon);

	//======================= Transform・各行列の初期化 ===========================//

//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//uvTransformの行列
	uvTransform_ = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::Inverse(cameraView);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f
	);
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
	ImGui::DragFloat2("UVTranslate", &uvTransform_.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat2("UVScale", &uvTransform_.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("UVRotate", &uvTransform_.rotate.z);
	MyMath::Normalize(directionalLightData_->direction_);
	Matrix4x4 uvTransformMatrix = MatrixMath::MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;
	ImGui::End();



#endif // _DEBUG
}

void Sphere::InitializeVertexData(DirectXCommon* dxCommon) {

	// 頂点数の設定
	uint32_t vertexCount = (kSubdivision * kSubdivision) * 6;

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 緯度分割1つ分の角度

	//VertexResource生成
	vertexResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount;
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
			vertexData_[start].normal.x = vertexData_[start].position.x;
			vertexData_[start].normal.y = vertexData_[start].position.y;
			vertexData_[start].normal.z = vertexData_[start].position.z;

			//基準点b1(左上)
			vertexData_[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData_[start + 1].position.y = sin(lat + kLatEvery);
			vertexData_[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData_[start + 1].position.w = 1.0f;
			vertexData_[start + 1].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData_[start + 1].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData_[start + 1].normal.x = vertexData_[start + 1].position.x;
			vertexData_[start + 1].normal.y = vertexData_[start + 1].position.y;
			vertexData_[start + 1].normal.z = vertexData_[start + 1].position.z;

			//基準点c1(右下)
			vertexData_[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData_[start + 2].position.y = sin(lat);
			vertexData_[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData_[start + 2].position.w = 1.0f;
			vertexData_[start + 2].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData_[start + 2].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData_[start + 2].normal.x = vertexData_[start + 2].position.x;
			vertexData_[start + 2].normal.y = vertexData_[start + 2].position.y;
			vertexData_[start + 2].normal.z = vertexData_[start + 2].position.z;

			//基準点b2(左上)
			//vertexData_[start + 3] = vertexData_[start + 1];

			//基準点d(右上)
			vertexData_[start + 3].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData_[start + 3].position.y = sin(lat + kLatEvery);
			vertexData_[start + 3].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData_[start + 3].position.w = 1.0f;
			vertexData_[start + 3].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData_[start + 3].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData_[start + 3].normal.x = vertexData_[start + 3].position.x;
			vertexData_[start + 3].normal.y = vertexData_[start + 3].position.y;
			vertexData_[start + 3].normal.z = vertexData_[start + 3].position.z;

			//基準点c2(右下)
			//vertexData_[start + 5] = vertexData_[start + 2];
		}
	}
}

void Sphere::InitializeMaterialData(DirectXCommon* dxCommon) {
	//マテリアル用リソース作成
	materialResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	//materialにデータを書き込む
	materialData_ = nullptr;
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//色を書き込む
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//UVTransform行列の初期化
	materialData_->uvTransform = MatrixMath::MakeIdentity4x4();
	//Lightingを有効にする
	materialData_->enableLighting = true;
}

void Sphere::InitializeDirectionalLightData(DirectXCommon* dxCommon) {

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
	directionalLightData_->intensity_ = 1.0f;
}

void Sphere::InitializeIndexBufferView(DirectXCommon* dxCommon) {

	// インデックスバッファのサイズを設定
	uint32_t indexCount = (kSubdivision * kSubdivision) * 6;
	indexResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * indexCount);

	// インデックスバッファビューの設定
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			// 最初の三角形のインデックス
			indexData[start] = start;         //左下
			indexData[start + 1] = start + 2; //左上
			indexData[start + 2] = start + 1; //右下

			// 二つ目の三角形のインデックス
			indexData[start + 3] = start + 2; //左上
			indexData[start + 4] = start + 3; //右上
			indexData[start + 5] = start + 1; //右下
		}
	}
}

void Sphere::InitializeCommandList(DirectXCommon* dxCommon, Texture* texture1, Texture* texture2) {

	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? texture2->GetTextureSrvHandleGPU() : texture1->GetTextureSrvHandleGPU());
	//Lighting用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//IBVの設定
	dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	// 描画！(DrawCall/ドローコール)
	dxCommon->GetCommandList()->DrawIndexedInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0, 0);
}

