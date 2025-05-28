#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "SrvManager.h"
#include "Material.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include <memory>

enum PrimitiveType {
	PRIMITIVE_RING,
	PRIMITIVE_PLANE,
	PRIMITIVE_COUNT
};

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
		float outerRadius_ = 1.0f; // 外側の半径
		float innerRadius_ = 0.01f; // 内側の半径
	};

	//plane全体のデータ
	struct PlaneData {
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

	void UpdateImGui();

	/// <summary>
	/// リングの描画
	/// </summary>
	/// <param name="outerRadius">外側の半径</param>
	/// <param name="innerRadius">内側の半径</param>
	/// <param name="center">中心点</param>
	/// <param name="color">カラー</param>
	void GenerateRing(const float outerRadius, const float innerRadius);

	void GeneratePlane(const float width, const float height);

	// 描画処理
	void DrawParticle(PSO* pso,UINT instanceCount,PrimitiveType type);

	void CreateVertexData(PrimitiveType type);

	// リングの頂点データの作成関数
	void CreateRingVertexData();

	void CreatePlaneVertexData();

	void CreateMatrialData(PrimitiveType type, const std::string& textureFilePath);

	void CreateRingMaterial(const std::string& textureFilePath);

	void CreatePlaneMaterial(const std::string& textureFilePath);

	void ResetVertexIndex() { ringVertexIndex_ = 0; }

	Material* GetRingMaterial() { return ringData_->material_; }

	Material* GetPlaneMaterial() { return planeData_->material_; }

private:

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	//std::unique_ptr<PSO> pso_;
	//ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;

	uint32_t srvIndex_ = 0;
	ComPtr<ID3D12Resource> particleResource_;
	ParticleForGPU* particleData_ = nullptr;
	ComPtr<ID3D12Resource> perviewResource_;
	PerView* perViewData_ = nullptr;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;
	EulerTransform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	RingData* ringData_ = nullptr;
	uint32_t ringDivide_ = 32;
	uint32_t ringVertexIndex_ = 0;
	uint32_t ringInstanceIndex_ = 0;
	uint32_t ringVertexCount_ = ringDivide_ * 6;

	PlaneData* planeData_ = nullptr;
	uint32_t planeVertexIndex_ = 0;
	uint32_t planeVertexCount_ = 0;

	const uint32_t kMaxVertexCount_ = 32000;
};

