#include "Mesh.h"
#include "engine/base/DirectXCommon.h"
#include "engine/base/TextureManager.h"
#include "engine/3d/ModelData.h"
#include <numbers>

//=============================================================================
// メッシュ初期化
//=============================================================================
void Mesh::InitializeMesh(TakeC::DirectXCommon* dxCommon,std::vector<ModelMaterialData> materialData) {

	for(auto& matData : materialData) {
		//マテリアル生成
		material_.emplace_back(std::make_unique<Material>());
		//マテリアル初期化
		material_.back()->Initialize(dxCommon,matData.textureFilePath,matData.envMapFilePath);
		//マテリアルの各種データ設定
		material_.back()->SetMaterialColor(matData.baseColor);
	}
	
	vertexBufferViews_.resize(1);
}

void Mesh::InitializeMesh(TakeC::DirectXCommon* dxCommon, const std::string& texturePath, const std::string& envMapPath) {

	//マテリアル生成
	material_.emplace_back(std::make_unique<Material>());
	//マテリアル初期化
	material_.back()->Initialize(dxCommon, texturePath, envMapPath);
	
	vertexBufferViews_.resize(1);
}


//============================================================================
// 描画処理時に使用する頂点バッファを設定
//============================================================================
void Mesh::SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot) {
	//頂点バッファビューの設定
	assert(vertexBufferViews_.size() > 0);
	commandList->IASetVertexBuffers(startSlot, static_cast<UINT>(vertexBufferViews_.size()), vertexBufferViews_.data());
}

//============================================================================
// スキニング後の頂点バッファを設定
//============================================================================
void Mesh::SetSkinnedVertexBuffer(ID3D12GraphicsCommandList* commandList, UINT startSlot) {

	commandList->IASetVertexBuffers(startSlot, 1, &skinnedVBV_);
}

//============================================================================
// 頂点バッファビュー追加
//============================================================================
void Mesh::AddVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vbv) {
	vertexBufferViews_.push_back(vbv);
}

//=============================================================================
// スプライトの頂点バッファリソース初期化
//=============================================================================
void Mesh::InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint) {


	//VertexResource生成
	inputVertexResource_ = TakeC::DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 4);
	//頂点バッファビューの作成
	vertexBufferViews_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//anchorPoint
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;
	//1枚目の三角形
	vertexData[0].position = { left,bottom,0.0f,1.0f }; //左下
	vertexData[1].position = { left,top,0.0f,1.0f }; //左上
	vertexData[2].position = { right,bottom,0.0f,1.0f }; //右下
	vertexData[3].position = { right,top,0.0f,1.0f }; //右上
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
}

//=============================================================================
// objモデルの頂点バッファリソース初期化
//=============================================================================

void Mesh::InitializeInputVertexResourceModel(ID3D12Device* device, ModelData* modelData) {


	//頂点リソースを作る
	inputVertexResource_ = TakeC::DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * modelData->vertices.size());
	

	//頂点バッファビューを作る
	vertexBufferViews_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//リソースにデータを書き込む
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());
}

//================================================================================================
// objモデルの頂点バッファリソース初期化（出力用）
//================================================================================================
void Mesh::InitializeOutputVertexResourceModel(ID3D12Device* device, ModelData* modelData) {
	//頂点リソースを作る
	outputVertexResource_ = TakeC::DirectXCommon::CreateBufferResourceUAV(device, sizeof(VertexData) * modelData->vertices.size());
	outputVertexResource_->SetName(L"Mesh::outputVertexResource_");
	//頂点バッファビューを作る
	vertexBufferViews_[0].BufferLocation = outputVertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);
}

//================================================================================================
// スプライトのIndexResource初期化
//================================================================================================
void Mesh::InitializeIndexResourceSprite(ID3D12Device* device) {

	//リソースの作成
	indexBuffer_ = TakeC::DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}

//================================================================================================
// モデルのIndexResourceの初期化
//================================================================================================

void Mesh::InitializeIndexResourceModel(ID3D12Device* device, ModelData* modelData) {

	indexBuffer_ = TakeC::DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * modelData->indices.size());
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData->indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, modelData->indices.data(), sizeof(uint32_t) * modelData->indices.size());
}

//================================================================================================
// モデルの頂点バッファリソースにデータをマップ
//================================================================================================
void Mesh::MapInputVertexResource(ModelData* modelData) {
	//リソースにデータを書き込む
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());
}
