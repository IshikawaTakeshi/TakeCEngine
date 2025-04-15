#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "SrvManager.h"
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

	// Ring全体のデータ
	struct RingData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
	};

public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	/// 終了処理
	void Finalize();

	//更新処理
	void Update();

	/// <summary>
	/// リングの描画
	/// </summary>
	/// <param name="outerRadius">外側の半径</param>
	/// <param name="innerRadius">内側の半径</param>
	/// <param name="center">中心点</param>
	/// <param name="color">カラー</param>
	void DrawRing(const float outerRadius, const float innerRadius, const Vector3& center, const Vector4& color);

	// 描画処理
	void Draw();

	// リングの頂点データの作成関数
	void CreateRingVertexData();



private:

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

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
	uint32_t ringVertexCount_ = ringDivide_;
	const uint32_t kMaxVertexCount_ = 32000;
};

