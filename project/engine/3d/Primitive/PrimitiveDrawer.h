#pragma once
#include "base/DirectXCommon.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "3d/Material.h"
#include "3d/VertexData.h"
#include "engine/3d/Primitive/Ring.h"
#include "engine/3d/Primitive/Plane.h"
#include "engine/3d/Primitive/Sphere.h"
#include "engine/3d/Primitive/Cube.h"
#include "engine/3d/Primitive/Cone.h"
#include "math/TransformMatrix.h"
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
namespace TakeC {

	class PrimitiveDrawer {
	public:

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		PrimitiveDrawer() = default;
		~PrimitiveDrawer() = default;

	public:

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
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

		/// 終了処理
		void Finalize();

		// ImGui更新処理
		void UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3& param);

		//リングデータの生成
		uint32_t GenerateRing(float outerRadius, float innerRadius, const std::string& textureFilePath);
		// 平面データの生成
		uint32_t GeneratePlane(float width, float height, const std::string& textureFilePath);
		// 球データの生成
		uint32_t GenerateSphere(float radius, const std::string& textureFilePath);
		// 円錐データの生成
		uint32_t GenerateCone(float radius, float height, uint32_t subDivision, const std::string& textureFilePath);
		//cubeデータの作成
		uint32_t GenerateCube(const AABB& size, const std::string& textureFilePath);

		// 描画処理(particle用)
		void DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle);
		// 描画処理(オブジェクト用)
		void DrawObject(PSO* pso, PrimitiveType type, uint32_t handle);

		Plane::PlaneData* GetPlaneData(uint32_t handle);
		Sphere::SphereData* GetSphereData(uint32_t handle);
		Ring::RingData* GetRingData(uint32_t handle);
		Cone::ConeData* GetConeData(uint32_t handle);
		Cube::CubeData* GetCubeData(uint32_t handle);

		void SetMaterialColor(uint32_t handle, PrimitiveType type, const Vector4& color);

	private:

		TakeC::DirectXCommon* dxCommon_ = nullptr;
		TakeC::SrvManager* srvManager_ = nullptr;

		TransformMatrix* TransformMatrixData_ = nullptr;
		EulerTransform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

		// プリミティブデータ管理用マップ
		//ring
		std::unique_ptr<TakeC::Ring> ring_ = nullptr;

		//plane
		std::unique_ptr<TakeC::Plane> plane_ = nullptr;

		//sphere
		std::unique_ptr<TakeC::Sphere> sphere_ = nullptr;

		//cone
		std::unique_ptr<TakeC::Cone> cone_ = nullptr;

		//cube
		std::unique_ptr<TakeC::Cube> cube_ = nullptr;

		const uint32_t kMaxVertexCount_ = 32000;
	};
}