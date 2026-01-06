#pragma once
#include "engine/3d/Primitive/PrimitiveBase.h"

namespace TakeC {

	//============================================================
	//	Plane class
	//============================================================

	class Plane : public PrimitiveBase<Plane::PlaneData> {
	public:
			//========================================================
			//	平面データ構造体
			//========================================================
			// Plane全体のデータ
			struct PlaneData : public PrimitiveBaseData  {
				float width = 1.0f;
				float height = 1.0f;

			};

			//========================================================
			//	functions
			//========================================================
			/// <summary>
			///	コンストラクタ・デストラクタ
			/// </summary>
			Plane() = default;
			~Plane() = default;

			uint32_t Generate(float width, float height, const std::string& textureFilePath);
	protected:
		//========================================================
		// オーバーライド
		//========================================================
		void CreateVertexData(PlaneData* planeData) override;
		void EditPrimitiveData(PlaneData* planeData) override;

		// 定数
		static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数
	};
}