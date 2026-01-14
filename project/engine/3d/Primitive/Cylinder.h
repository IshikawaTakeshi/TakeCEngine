#pragma once
#include "engine/3d/Primitive/PrimitiveBase.h"

namespace TakeC {

	//========================================================
	//cylinderのデータ
	//========================================================
	struct CylinderData : public PrimitiveBaseData {
		float radius = 1.0f;
		float height = 1.0f;
		uint32_t subDivision = 16;
	};

	//============================================================
	//	Cylinder class
	//============================================================
	class Cylinder : public PrimitiveBase<CylinderData> {
	public:

		//データ型エイリアス
		using DataType = CylinderData;

		//========================================================
		//	functions
		//========================================================
		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Cylinder() = default;
		~Cylinder() = default;
		/// <summary>
		/// cylinderデータの作成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(float radius, float height, uint32_t subDivision, const std::string& textureFilePath);

		/// <summary>
		/// 頂点データ作成
		/// </summary>
		/// <param name="cylinderData"></param>
		void CreateVertexData(CylinderData* cylinderData) override;

	protected:

		/// <summary>
		/// プリミティブデータ編集
		/// </summary>
		/// <param name="data"></param>
		void EditPrimitiveData(CylinderData* data) override;
	};
}