#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "Mesh/Mesh.h"
#include "engine/3d/Primitive/PrimitiveBase.h"
#include "engine/Base/SrvManager.h"
#include "math/AABB.h"

#include <memory>
#include <unordered_map>
#include <string>

namespace TakeC {

	//============================================================
	//	Cube class
	//============================================================

	class Cube {
	public:

		//========================================================
		//	cubeデータ構造体
		//========================================================
		// Cube全体のデータ
		struct CubeData {
			PrimitiveMesh mesh;
			VertexData* vertexData = nullptr;
			std::unique_ptr<Material> material = nullptr;
			AABB size;
			uint32_t vertexCount = 0;
		};
		//========================================================
		//	functions
		//========================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Cube() = default;
		~Cube() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="dxCommon"></param>
		/// <param name="srvManager"></param>
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

		/// <summary>
		/// cubeデータの作成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(const AABB& size, const std::string& textureFilePath);

		/// <summary>
		/// 描画処理(パーティクル用)
		/// </summary>
		/// <param name="pso"></param>
		/// <param name="instanceCount"></param>
		/// <param name="handle"></param>
		void DrawParticle(PSO* pso, UINT instanceCount, uint32_t handle);

		/// <summary>
		/// 描画処理（オブジェクト用）
		/// </summary>
		void DrawObject(PSO* pso, uint32_t handle);

		//========================================================================
		// accessors
		//========================================================================
		/// データ取得
		CubeData* GetData(uint32_t handle);
		/// マテリアル色設定
		void SetMaterialColor(uint32_t handle, const Vector4& color);

	private:

		//========================================================================
		// private functions
		//========================================================================
		/// 頂点データ作成
		void CreateVertexData(CubeData* planeData);
		/// 頂点インデックスデータ作成
		//void CreateVertexIndexData(CubeData* planeData);

		/// マテリアル作成
		void CreateMaterial(const std::string& textureFilePath, CubeData* planeData);

	private:

		//========================================================================
		// member variables
		//========================================================================

		// DirectXCommonへのポインタ
		DirectXCommon* dxCommon_ = nullptr;
		// SrvManagerへのポインタ
		SrvManager* srvManager_ = nullptr;
		// プリミティブデータ管理用マップ
		std::unordered_map<uint32_t, std::unique_ptr<CubeData>> datas_;
		// ハンドル生成用カウンタ
		uint32_t nextHandle_ = 0;
		//頂点数
		static constexpr uint32_t kVertexCount = 36;
		//頂点インデックス数
		static constexpr uint32_t kIndexCount = 36;
	};
}