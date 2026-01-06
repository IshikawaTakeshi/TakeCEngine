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
	//	Cone class
	//============================================================

	class Cone {
		
		public:
		//========================================================
		//	coneデータ構造体
		//========================================================
		// Cone全体のデータ
		struct ConeData {
			PrimitiveMesh mesh;
			VertexData* vertexData = nullptr;
			std::unique_ptr<Material> material = nullptr;
			float radius = 0.0f;
			float height = 0.0f;
			uint32_t subDivision = 32; // 分割数
			uint32_t vertexCount = 0;
		};
		//========================================================
		//	functions
		//========================================================
		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Cone() = default;
		~Cone() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="dxCommon"></param>
		/// <param name="srvManager"></param>
		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager);
		/// <summary>
		/// coneデータの作成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(float radius, float height, uint32_t subDivision, const std::string& textureFilePath);
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
		ConeData* GetData(uint32_t handle);
		/// マテリアル色設定
		void SetMaterialColor(uint32_t handle, const Vector4& color);

	private:

		//========================================================
		// private functions
		//========================================================

		//頂点データ作成
		void CreateVertexData(ConeData* coneData);
		//マテリアル作成関数
		void CreateMaterial(const std::string& textureFilePath, ConeData* coneData);


	private:

		// DirectXCommonへのポインタ
		DirectXCommon* dxCommon_ = nullptr;
		// SrvManagerへのポインタ
		SrvManager* srvManager_ = nullptr;
		// coneデータマップ
		std::unordered_map<uint32_t, std::unique_ptr<ConeData>> datas_;
		// 次のハンドル値
		uint32_t nextHandle_ = 0;
	};

}