#include "SkinMesh.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include <numbers>

SkinMesh::~SkinMesh() {
	material_.reset();
	vertexResource_.Reset();
	indexResource_.Reset();
}

void SkinMesh::SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot) {
	assert(vertexBufferViews_.size() > 0);
	commandList->IASetVertexBuffers(startSlot, static_cast<UINT>(vertexBufferViews_.size()), vertexBufferViews_.data());
}

void SkinMesh::InitializeVertexResourceModel(ID3D12Device* device, ModelData modelData) {
	//頂点リソースを作る
	vertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作る
	vertexBufferViews_[0].BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//リソースにデータを書き込む
	VertexData* vertexData;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

void SkinMesh::InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData) {

	indexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * modelData.indices.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData.indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());
}
