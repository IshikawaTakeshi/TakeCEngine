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
	// リングデータ構造体
	//============================================================

	// Ring全体のデータ
	struct RingData : public PrimitiveBaseData {
		float outerRadius = 1.0f; // 外側の半径
		float innerRadius = 0.01f; // 内側の半径
		uint32_t subDivision = 32; // 分割数
	};

	//============================================================
	//	Ring class
	//============================================================

	class Ring : public PrimitiveBase<RingData> {
	public:

		//========================================================
		// functions
		//========================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		Ring() = default;
		~Ring() = default;

		/// <summary>
		/// リングデータの生成
		/// </summary>
		/// <returns>生成したハンドル</returns>
		uint32_t Generate(float outerRadius, float innerRadius, const std::string& textureFilePath);


	protected:

		//========================================================================
		// private functions
		//========================================================================
		/// 頂点データ作成
		void CreateVertexData(RingData* ringData);
		
		void EditPrimitiveData(RingData* data);

	private:

		// 定数
		static constexpr uint32_t kVerticesPerSegment = 6;  // 1セグメントあたりの頂点数
	};
}
