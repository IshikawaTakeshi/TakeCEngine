#pragma once
#include "engine/3d/Primitive/PrimitiveBase.h"
#include "math/AABB.h"

namespace TakeC {

	//========================================================
	//	cubeデータ構造体
	//========================================================
	// Cube全体のデータ
	struct CubeData : public PrimitiveBaseData {
		AABB size;
	};

	//============================================================
	//	Cube class
	//============================================================

	class Cube : public PrimitiveBase<CubeData> {
	public:

		//データ型エイリアス
		using DataType = CubeData;

		//========================================================
		//	functions
		//========================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Cube() = default;
		~Cube() = default;

		/// <summary>
		/// cubeデータの作成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(const AABB& size, const std::string& textureFilePath);
		/// 頂点データ作成
		void CreateVertexData(CubeData* cubeData) override;

	protected:

		//========================================================================
		// private functions
		//========================================================================
		
		void EditPrimitiveData(CubeData* data) override;

	private:

		//頂点数
		static constexpr uint32_t kVertexCount = 36;
		//頂点インデックス数
		static constexpr uint32_t kIndexCount = 36;
	};
}