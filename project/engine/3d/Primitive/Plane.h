#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "Mesh/Mesh.h"
#include "engine/3d/Primitive/PrimitiveBase.h"
#include "engine/Base/SrvManager.h"

#include <memory>
#include <unordered_map>
#include <string>

namespace TakeC {

	//============================================================
	//	Plane class
	//============================================================

	class Plane {
	public:
			//========================================================
			//	平面データ構造体
			//========================================================
			// Plane全体のデータ
			struct PlaneData {
				PrimitiveMesh mesh;
				VertexData* vertexData = nullptr;
				std::unique_ptr<Material> material = nullptr;
				float width = 1.0f;
				float height = 1.0f;
				uint32_t vertexCount = 0;
			};

			//========================================================
			//	functions
			//========================================================
			/// <summary>
			///	コンストラクタ・デストラクタ
			/// </summary>
			Plane() = default;
			~Plane() = default;

			void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);

			/// <summary>
			/// plane生成
			/// </summary>
			/// <returns>生成したハンドル</returns>
			uint32_t Generate(float width, float height, const std::string& textureFilePath);

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
			PlaneData* GetData(uint32_t handle);
			/// マテリアル色設定
			void SetMaterialColor(uint32_t handle, const Vector4& color);

	private:

		//========================================================================
		// private functions
		//========================================================================
		/// 頂点データ作成
		void CreateVertexData(PlaneData* planeData);
		/// マテリアル作成
		void CreateMaterial(const std::string& textureFilePath, PlaneData* planeData);

	private:

		//========================================================================
		// member variables
		//========================================================================

		// DirectXCommonへのポインタ
		DirectXCommon* dxCommon_ = nullptr;
		// SrvManagerへのポインタ
		SrvManager* srvManager_ = nullptr;
		// プリミティブデータ管理用マップ
		std::unordered_map<uint32_t, std::unique_ptr<PlaneData>> datas_;
		// ハンドル生成用カウンタ
		uint32_t nextHandle_ = 0;

		// 定数
		static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数
	};
}