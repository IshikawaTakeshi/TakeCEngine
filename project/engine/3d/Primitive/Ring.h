#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "Mesh/Mesh.h"
#include "engine/3d/Primitive/PrimitiveMesh.h"
#include "engine/Base/SrvManager.h"

#include <memory>
#include <unordered_map>
#include <string>

//============================================================
//	Ring class
//============================================================
namespace TakeC {

	class DirectXCommon;

	class Ring {
	public:

		//========================================================
		// リングデータ構造体
		//========================================================

		// Ring全体のデータ
		struct RingData {
			PrimitiveMesh mesh;
			VertexData* vertexData = nullptr;
			std::unique_ptr<Material> material = nullptr;
			float outerRadius = 1.0f; // 外側の半径
			float innerRadius = 0.01f; // 内側の半径
			uint32_t subDivision = 32; // 分割数
			uint32_t vertexCount = 0; // このインスタンスの頂点数
		};

		//========================================================
		// functions
		//========================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Ring() = default;
		~Ring() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="dxCommon"></param>
		void Initialize(TakeC::DirectXCommon* dxCommon,TakeC::SrvManager* srvManager);

		/// <summary>
		/// リングデータの生成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(float outerRadius, float innerRadius, const std::string& textureFilePath);

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
		RingData* GetData(uint32_t handle);
		/// マテリアル色設定
		void SetMaterialColor(uint32_t handle, const Vector4& color);

	private:

		//========================================================================
		// private functions
		//========================================================================
		/// 頂点データ作成
		void CreateVertexData(RingData* ringData);
		/// マテリアル作成
		void CreateMaterial(const std::string& textureFilePath, RingData* ringData);

	private:

		//========================================================================
		// member variables
		//========================================================================

		DirectXCommon* dxCommon_ = nullptr;
		SrvManager* srvManager_ = nullptr;
		// プリミティブデータ管理用マップ
		std::unordered_map<uint32_t, std::unique_ptr<RingData>> datas_;
		uint32_t nextHandle_ = 0;

		// 定数
		static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数
	};
}
