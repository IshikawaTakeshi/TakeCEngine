#include "Sprite.h"
#include "DirectXCommon.h"
#include "MyMath/MatrixMath.h"
#include "DirectXCommon.h"

#pragma region 初期化処理
void Sprite::Initialize(DirectXCommon* dxCommon) {

	//スプライト用VertexResource生成
	vertexResourceSprite_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 6);
	//頂点バッファビューの作成
	vertexBufferViewSprite_.BufferLocation = vertexResourceSprite_->GetGPUVirtualAddress();
	vertexBufferViewSprite_.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite_.StrideInBytes = sizeof(VertexData);


	vertexResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite_));
	//1枚目の三角形
	vertexDataSprite_[0].position = { 0.0f,360.0f,0.0f,1.0f }; //左下
	vertexDataSprite_[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite_[1].position = { 0.0f,0.0f,0.0f,1.0f }; //左上
	vertexDataSprite_[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite_[2].position = { 640.0f,360.0f,0.0f,1.0f }; //右下
	vertexDataSprite_[2].texcoord = { 1.0f,1.0f };
	//2枚目の三角形
	vertexDataSprite_[3].position = { 0.0f,0.0f,0.0f,1.0f }; //左上
	vertexDataSprite_[3].texcoord = { 0.0f,0.0f };
	vertexDataSprite_[4].position = { 640.0f,0.0f,0.0f,1.0f }; //右上
	vertexDataSprite_[4].texcoord = { 1.0f,0.0f };
	vertexDataSprite_[5].position = { 640.0f,360.0f,0.0f,1.0f }; //右下
	vertexDataSprite_[5].texcoord = { 1.0f,1.0f };


	//スプライト用のTransformationMatrix用のVertexResource生成
	transformationMatrixResourceSprite_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));

	//TransformationMatrix用
	transformationMatrixResourceSprite_->
		Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite_));

	//単位行列を書き込んでおく
	*transformationMatrixDataSprite_ = MatrixMath::MakeIdentity4x4();

	//CPUで動かす用のTransform
	transformSprite_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	worldMatrixSprite_ = MatrixMath::MakeAffineMatrix(
		transformSprite_.scale,
		transformSprite_.rotate,
		transformSprite_.translate
	);


	viewMatrixSprite_ = MatrixMath::MakeIdentity4x4();

	projectionMatrixSprite_ = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	worldViewProjectionMatrixSprite_ = MatrixMath::Multiply(
		worldMatrixSprite_, MatrixMath::Multiply(viewMatrixSprite_, projectionMatrixSprite_));
	*transformationMatrixDataSprite_ = worldViewProjectionMatrixSprite_;
}
#pragma endregion

#pragma region 更新処理
void Sprite::Update() {

	worldMatrixSprite_ = MatrixMath::MakeAffineMatrix(
		transformSprite_.scale,
		transformSprite_.rotate,
		transformSprite_.translate
	);

	worldViewProjectionMatrixSprite_ = MatrixMath::Multiply(
		worldMatrixSprite_, MatrixMath::Multiply(viewMatrixSprite_, projectionMatrixSprite_));
	*transformationMatrixDataSprite_ = worldViewProjectionMatrixSprite_;

}
#pragma endregion

#pragma region resource生成関数
Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(
	Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT result = S_FALSE;
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	//頂点リソースの設定
	//リソースディスク
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes; // リソースのサイズ。今回はVector4を3頂点分
	//バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));

	return resource;
}
#pragma endregion