#pragma once
#include "engine/3d/Primitive/PrimitiveBase.h"

namespace TakeC {

	//========================================================
	//	coneデータ構造体
	//========================================================
	// Cone全体のデータ
	struct ConeData : public PrimitiveBaseData {
		float radius = 0.0f;
		float height = 0.0f;
		uint32_t subDivision = 32; // 分割数
	};

	//============================================================
	//	Cone class
	//============================================================

	class Cone : public PrimitiveBase<ConeData> {	
	public:

		//データ型エイリアス
		using DataType = ConeData;

		//========================================================
		//	functions
		//========================================================
		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Cone() = default;
		~Cone() = default;

		/// <summary>
		/// coneデータの作成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(float radius, float height, uint32_t subDivision, const std::string& textureFilePath);

	public:
		/// <summary>
		/// 頂点データ作成
		/// </summary>
		/// <param name="coneData"></param>
		void CreateVertexData(ConeData* coneData);

	protected:
		
		/// <summary>
		/// プリミティブデータ編集
		/// </summary>
		/// <param name="data"></param>
		void EditPrimitiveData(ConeData* data) override;

	};
}