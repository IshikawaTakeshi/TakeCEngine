#pragma once
#include "engine/3d/Primitive/PrimitiveBase.h"

namespace TakeC {

	//========================================================
	// sphere情報の構造体
	//========================================================

	struct SphereData : public PrimitiveBaseData {
		float radius = 1.0f; // 半径
		uint32_t subDivision = 16; // 分割数
	};

	//============================================================
	//	Sphere class
	//============================================================

	class Sphere : public PrimitiveBase<SphereData> {
	public:

		//データ型エイリアス
		using DataType = SphereData;

		//========================================================
		// functions
		//========================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Sphere() = default;
		~Sphere() = default;

		/// <summary>
		/// リングデータの生成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(float radius,uint32_t subDivision, const std::string& textureFilePath);

		/// <summary>
		/// 頂点データ作成
		/// </summary>
		/// <param name="sphereData"></param>
		void CreateVertexData(SphereData* sphereData);
	protected:

		/// <summary>
		/// プリミティブデータ編集
		/// </summary>
		/// <param name="data"></param>
		void EditPrimitiveData(SphereData* data);


	private:

		// 定数
		static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数

	};
}