#pragma once
#include "engine/AI/OnnxModel.h"

namespace TakeC {

	class DirectXCommon;

	//====================================================================
	// 	OnnxRuntimeSystem class
	//====================================================================
	class OnnxRuntimeSystem {
	public:
		
		//================================================================
		// functions
		//================================================================

		/// <summary>
		/// インスタンスの取得
		/// </summary>
		/// <returns></returns>
		static OnnxRuntimeSystem& GetInstance();

		void Initialize(DirectXCommon* dxCommon);

		void Finalize();

		//OnnxModel* LoadModel
	};

}