#pragma once

//============================================================
//	GBufferTypeEnum
//============================================================

namespace TakeC {
	enum GBufferTypeEnum {
		// アルベド
		GBUFFER_Albedo,
		// 法線
		GBUFFER_Normal,
		//マテリアル
		GBUFFER_Material,
		//環境マップインデックス
		GBUFFER_EnvMapIndex,
		// カウンター
		kNumGBuffers,
	};
}