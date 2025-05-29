#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "SrvManager.h"
#include "Material.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include <memory>
#include <cstdint>
#include <unordered_map>

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
		float width_;
		float height_;
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
	uint32_t GenerateRing(const float outerRadius, const float innerRadius, const std::string& textureFilePath);

	uint32_t GeneratePlane(const float width, const float height, const std::string& textureFilePath);

	// 描画処理
	void DrawParticle(PSO* pso,UINT instanceCount,PrimitiveType type,uint32_t handle);

	//void CreateVertexData(PrimitiveType type);

	// リングの頂点データの作成関数
	void CreateRingVertexData(RingData* ringData);

	void CreatePlaneVertexData(PlaneData* planeData);

	//void CreateMatrialData(PrimitiveType type, const std::string& textureFilePath);

	void CreateRingMaterial(const std::string& textureFilePath,RingData* ringData);

	void CreatePlaneMaterial(const std::string& textureFilePath,PlaneData* planeData);

	void ResetVertexIndex() { ringVertexIndex_ = 0; }

	/*Material* GetRingMaterial() { return ringData_->material_; }

	Material* GetPlaneMaterial() { return planeData_->material_; }*/

private:

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	TransformMatrix* TransformMatrixData_ = nullptr;
	EulerTransform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	std::unordered_map<uint32_t,std::unique_ptr<RingData>> ringDatas_;
	uint32_t ringDivide_ = 32;
	uint32_t ringVertexIndex_ = 0;
	uint32_t ringVertexCount_ = 0;
	uint32_t ringHandle_ = 0;

	std::unordered_map<uint32_t,std::unique_ptr<PlaneData>> planeDatas_;
	uint32_t planeVertexIndex_ = 0;
	uint32_t planeVertexCount_ = 0;
	uint32_t planeHandle_ = 0;

	const uint32_t kMaxVertexCount_ = 32000;
};

