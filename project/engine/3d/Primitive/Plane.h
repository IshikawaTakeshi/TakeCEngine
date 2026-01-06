#pragma once
#include "engine/3d/Primitive/PrimitiveBase.h"

namespace TakeC {

	//========================================================
	//	平面データ構造体
	//========================================================
	// Plane全体のデータ
	struct PlaneData : public PrimitiveBaseData {
		float width = 1.0f;
		float height = 1.0f;

	};

	//============================================================
	//	Plane class
	//============================================================

	class Plane : public PrimitiveBase<PlaneData> {
	public:


		//========================================================
		//	functions
		//========================================================
		/// <summary>
		///	コンストラクタ・デストラクタ
		/// </summary>
		Plane() = default;
		~Plane() = default;

		/// <summary>
		/// planeデータの生成
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="textureFilePath"></param>
		/// <returns></returns>
		uint32_t Generate(float width, float height, const std::string& textureFilePath);

	protected:
		
		/// <summary>
		/// 頂点データ作成
		/// </summary>
		/// <param name="planeData"></param>
		void CreateVertexData(PlaneData* planeData) override;

		/// <summary>
		/// プリミティブデータ編集
		/// </summary>
		/// <param name="planeData"></param>
		void EditPrimitiveData(PlaneData* planeData) override;

		// 定数
		static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数
	};
}