#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "Material.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include <memory>

struct PrimitiveMesh {
	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	ComPtr<ID3D12Resource> indexResource_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
};

class PrimitiveDrawer {
public:
	PrimitiveDrawer() = default;
	~PrimitiveDrawer() = default;
	
public:

	//Ring全体のデータ
	struct RingData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
	};

public:

	void Initialize(DirectXCommon* dxCommon);
	void Update();

	void DrawRing(const float outerRadius, const float innerRadius, const Vector3& center, const Vector4& color);

	void Draw();

	void CreateRingVertexData();



private:

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<PSO> pso_;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	RingData* ringData_ = nullptr;
	uint32_t ringDivide_ = 32;
	uint32_t ringVertexIndex_ = 0;
	uint32_t ringIndexIndex_ = 0;
	uint32_t ringVertexCount_ = 0;
	const uint32_t kMaxVertexCount_ = 32000;
};

