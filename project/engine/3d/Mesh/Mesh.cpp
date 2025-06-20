#include "Mesh.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "3d/Model.h"
#include <numbers>
#include <cassert>

ModelMesh::~ModelMesh() {
	subMeshes_.clear();
	inputVertexResource_.Reset();
	outputVertexResource_.Reset();
	indexResource_.Reset();
}

void ModelMesh::InitializeVertexResourceSphere(ID3D12Device* device) {

	// 頂点数の設定
	uint32_t vertexCount = (kSubdivision * kSubdivision) * 6;

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 緯度分割1つ分の角度

	//VertexResource生成
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	VBVs_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	VBVs_[0].SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	VBVs_[0].StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	// 書き込むためのアドレスを取得
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; //現在の緯度(シータ)

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; //現在の経度(ファイ)
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			//1枚目の三角形
			//頂点データの入力。基準点a(左下)
			vertexData[start].position.x = cos(lat) * cos(lon);
			vertexData[start].position.y = sin(lat);
			vertexData[start].position.z = cos(lat) * sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData[start].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;

			//基準点b1(左上)
			vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData[start + 1].position.y = sin(lat + kLatEvery);
			vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData[start + 1].position.w = 1.0f;
			vertexData[start + 1].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData[start + 1].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
			vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
			vertexData[start + 1].normal.z = vertexData[start + 1].position.z;

			//基準点c1(右下)
			vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData[start + 2].position.y = sin(lat);
			vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData[start + 2].position.w = 1.0f;
			vertexData[start + 2].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData[start + 2].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
			vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
			vertexData[start + 2].normal.z = vertexData[start + 2].position.z;

			//基準点d(右上)
			vertexData[start + 3].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData[start + 3].position.y = sin(lat + kLatEvery);
			vertexData[start + 3].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData[start + 3].position.w = 1.0f;
			vertexData[start + 3].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData[start + 3].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
			vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
			vertexData[start + 3].normal.z = vertexData[start + 3].position.z;

		}
	}

}

void ModelMesh::InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint) {


	//VertexResource生成
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 4);
	//頂点バッファビューの作成
	VBVs_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	VBVs_[0].SizeInBytes = sizeof(VertexData) * 4;
	VBVs_[0].StrideInBytes = sizeof(VertexData);

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

void ModelMesh::InitializeVertexResourceTriangle(ID3D12Device* device) {

	//VertexResource生成
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 3);

	// リソースの先頭のアドレスから使う
	VBVs_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	VBVs_[0].SizeInBytes = sizeof(VertexData) * 3;
	// 1頂点あたりのサイズ
	VBVs_[0].StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	// 書き込むためのアドレスを取得
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

}

//=============================================================================
// objモデルの頂点バッファリソース初期化
//=============================================================================

void ModelMesh::InitializeInputVertexResourceModel(ID3D12Device* device) {


	//頂点リソースを作る
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * allVertices.size());
	
	//リソースにデータを書き込む
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, allVertices.data(), sizeof(VertexData) * allVertices.size());
}

void ModelMesh::InitializeOutputVertexResourceModel(ID3D12Device* device,ID3D12GraphicsCommandList* commandList) {
	//頂点リソースを作る
	outputVertexResource_ = DirectXCommon::CreateBufferResourceUAV(device, sizeof(VertexData) * allVertices.size(),commandList);
	outputVertexResource_->SetName(L"Mesh::outputVertexResource_");
}

void ModelMesh::InitializeIndexResourceSphere(ID3D12Device* device) {

	// インデックスバッファのサイズを設定
	uint32_t indexCount = (kSubdivision * kSubdivision) * 6;
	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * indexCount);

	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			// 最初の三角形のインデックス
			indexData[start] = start;         //左下
			indexData[start + 1] = start + 1; //左上
			indexData[start + 2] = start + 2; //右下

			// 二つ目の三角形のインデックス
			indexData[start + 3] = start + 1; //左上
			indexData[start + 4] = start + 3; //右上
			indexData[start + 5] = start + 2; //右下
		}
	}
}

void ModelMesh::InitializeIndexResourceSprite(ID3D12Device* device) {

	//リソースの作成
	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	IBVs_[0].BufferLocation = indexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	IBVs_[0].SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	IBVs_[0].Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}

//================================================================================================
// モデルのIndexResourceの初期化
//================================================================================================

void ModelMesh::InitializeIndexResourceModel(ID3D12Device* device) {

	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * allIndices.size());

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, allIndices.data(), sizeof(uint32_t) * allIndices.size());
}

void ModelMesh::MapInputVertexResource() {
	//リソースにデータを書き込む
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, allVertices.data(), sizeof(VertexData) * allVertices.size());
}

std::vector<D3D12_VERTEX_BUFFER_VIEW> ModelMesh::CreateSubMeshVBVs() const {
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs;
	for (const auto& sub : subMeshes_) {
		D3D12_VERTEX_BUFFER_VIEW vbv = {};
		vbv.BufferLocation = inputVertexResource_->GetGPUVirtualAddress() + sub.vertexStart * sizeof(VertexData);
		vbv.SizeInBytes = sub.vertexCount * sizeof(VertexData);
		vbv.StrideInBytes = sizeof(VertexData);
		vbvs.push_back(vbv);
	}
	return vbvs;
}

std::vector<D3D12_INDEX_BUFFER_VIEW> ModelMesh::CreateSubMeshIBVs() const {
	std::vector<D3D12_INDEX_BUFFER_VIEW> ibvs;
	for (const auto& sub : subMeshes_) {
		D3D12_INDEX_BUFFER_VIEW ibv = {};
		ibv.BufferLocation = indexResource_->GetGPUVirtualAddress() + sub.indexStart * sizeof(uint32_t);
		ibv.SizeInBytes = sub.indexCount * sizeof(uint32_t);
		ibv.Format = DXGI_FORMAT_R32_UINT;
		ibvs.push_back(ibv);
	}
	return ibvs;
}
