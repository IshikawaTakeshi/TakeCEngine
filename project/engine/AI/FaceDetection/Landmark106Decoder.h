#pragma once

#include <cstddef>
#include <vector>

#include "engine/Math/Vector2.h"

namespace TakeC {

	//==============================================================================================
	//		Landmark106Decoder class
	//==============================================================================================
	class Landmark106Decoder {
	public:

		static constexpr size_t kLandmarkCount = 106;

		struct Config {
			bool outputMinusOneToOne = true;
		};

		/// <summary>
		/// 2d106det形式の出力を位置合わせ済み顔画像上の座標へ変換する
		/// </summary>
		static std::vector<Vector2> Decode(
			const std::vector<std::vector<float>>& outputDataList,
			int alignedWidth,
			int alignedHeight,
			const Config& config = {});

	};

}
