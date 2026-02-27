#pragma once

//============================================================
//	GBufferTypeEnum
//============================================================

namespace TakeC {
	enum GBufferTypeEnum {
		// 法線
		GBUFFER_Normal,
		// アルベド
		GBUFFER_Albedo,
		//マテリアル
		GBUFFER_Material,
		// カウンター
		kNumGBuffers,
	};
}