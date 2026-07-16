#pragma once

#include <optional>
#include <vector>

#include "FacePartRatios.h"
#include "engine/Math/Vector2.h"

namespace TakeC {

	//==============================================================================================
	//		FacePartRatioCalculator class
	//==============================================================================================
	class FacePartRatioCalculator {
	public:

		/// <summary>
		/// InsightFace 2d106detの106点から顔パーツ比率を計算する
		/// </summary>
		static std::optional<FacePartRatios> Calculate(
			const std::vector<Vector2>& landmarks);

	};

}
