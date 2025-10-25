#pragma once
#include "base/DirectXCommon.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "3d/Material.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include "Primitive/PrimitiveType.h"
#include <memory>
#include <cstdint>
#include <unordered_map>

// プリミティブメッシュ構造体
struct PrimitiveMesh {
	ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
};


//============================================================================
// PrimitiveDrawer class
//============================================================================
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
		uint32_t subDivision_ = 32; // 分割数
	};

	//plane全体のデータ
	struct PlaneData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float width_;
		float height_;
	};

	//sphere全体のデータ
	struct SphereData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float radius_;
		uint32_t subDivision_ = 16; // 分割数
	};

	//cone全体のデータ
	struct ConeData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float radius_;
		float height_;
		uint32_t subDivision_ = 32; // 分割数
	};

	//cube全体のデータ
	struct CubeData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float size_;
	};

	//cylinder全体のデータ
	struct CylinderData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float radius_;
		float height_;
	};

public:

	//=================================================================================
	// functions
	//=================================================================================
	
	// 初期化
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	/// 終了処理
	void Finalize();

	//更新処理
	void Update();

	// ImGui更新処理
	void UpdateImGui(uint32_t handle,PrimitiveType type);
	void UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3& param);

	//リングデータの生成
	uint32_t GenerateRing(const float outerRadius, const float innerRadius, const std::string& textureFilePath);
	// 平面データの生成
	uint32_t GeneratePlane(const float width, const float height, const std::string& textureFilePath);
	// 球データの生成
	uint32_t GenerateSphere(const float radius, const std::string& textureFilePath);
	// 円錐データの生成
	uint32_t GenerateCone(const float radius, const float height,uint32_t subDivision, const std::string& textureFilePath);
	// 描画処理(particle用)
	void DrawParticle(PSO* pso,UINT instanceCount,PrimitiveType type,uint32_t handle);
	// 描画処理(オブジェクト用)
	void DrawAllObject(PSO* pso, PrimitiveType type, uint32_t handle);

	PlaneData* GetPlaneData(uint32_t handle);
	SphereData* GetSphereData(uint32_t handle);
	RingData* GetRingData(uint32_t handle);
	ConeData* GetConeData(uint32_t handle);

private:

	// リングの頂点データの作成関数
	void CreateRingVertexData(RingData* ringData);
	// 平面の頂点データの作成関数
	void CreatePlaneVertexData(PlaneData* planeData);
	// 球の頂点データの作成関数
	void CreateSphereVertexData(SphereData* sphereData);
	// Cube
	//void CreateCubeVertexData(CubeData* cubeData);
	// Cone
	void CreateConeVertexData(ConeData* coneData);

	// マテリアルの作成関数(リング、平面、球、円錐)
	void CreateRingMaterial(const std::string& textureFilePath,RingData* ringData);
	void CreatePlaneMaterial(const std::string& textureFilePath,PlaneData* planeData);
	void CreateSphereMaterial(const std::string& textureFilePath, SphereData* sphereData);
	void CreateConeMaterial(const std::string& textureFilePath, ConeData* coneData);

private:

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	TransformMatrix* TransformMatrixData_ = nullptr;
	EulerTransform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	// プリミティブデータ管理用マップ
	//ring
	std::unordered_map<uint32_t,std::unique_ptr<RingData>> ringDatas_;
	uint32_t ringDivide_ = 32;
	uint32_t ringVertexIndex_ = 0;
	uint32_t ringVertexCount_ = 0;
	uint32_t ringHandle_ = 0;

	//plane
	std::unordered_map<uint32_t,std::unique_ptr<PlaneData>> planeDatas_;
	uint32_t planeVertexIndex_ = 0;
	uint32_t planeVertexCount_ = 0;
	uint32_t planeHandle_ = 0;

	//sphere
	std::unordered_map<uint32_t, std::unique_ptr<SphereData>> sphereDatas_;
	uint32_t sphereDivide_ = 16; // 球の分割数
	uint32_t sphereVertexIndex_ = 0;
	uint32_t sphereVertexCount_ = 0;
	uint32_t sphereIndexCount_ = 0;
	uint32_t sphereHandle_ = 0;

	//cone
	std::unordered_map<uint32_t, std::unique_ptr<ConeData>> coneDatas_;
	uint32_t coneVertexIndex_ = 0;
	uint32_t coneVertexCount_ = 0;
	uint32_t coneHandle_ = 0;

	const uint32_t kMaxVertexCount_ = 32000;
};