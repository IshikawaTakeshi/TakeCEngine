#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "ResourceDataStructure.h"
#include <memory>

struct PrimitiveData {
	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	VertexData* vertexData_ = nullptr;
	ComPtr<ID3D12Resource> indexResource_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};

};

class PrimitiveDrawer {
public:
	PrimitiveDrawer() = default;
	~PrimitiveDrawer() = default;
	void Initialize(DirectXCommon* dxCommon);
	void Update();
	void Draw();

	void CreateRing(const uint32_t divide,const float outerRadius, const float innerRadius, const Vector3& center, const Vector4& color);

private:

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<PSO> pso_;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	uint32_t ringVertexIndex_ = 0;
	uint32_t ringIndex_ = 0;
	const uint32_t kMaxVertexCount_ = 10240;
};

