#pragma once

#include <cstdint>
#include <vector>

namespace TakeC {

	class OnnxModel;

	namespace OnnxImageTensorUtility {

		/// <summary>
		/// ONNXモデルの画像入力shapeに含まれるdynamic次元を既定値で具体化する
		/// </summary>
		bool ResolveRuntimeInputShape(
			const std::vector<int64_t>& modelShape,
			std::vector<int64_t>& runtimeShape);

		/// <summary>
		/// ONNXモデルの画像入力shapeに含まれるdynamic次元を指定サイズで具体化する
		/// </summary>
		bool ResolveRuntimeInputShape(
			const std::vector<int64_t>& modelShape,
			int fallbackWidth,
			int fallbackHeight,
			std::vector<int64_t>& runtimeShape);

		/// <summary>
		/// NCHWまたはNHWCの画像入力shapeから画像サイズを取得する
		/// </summary>
		bool ResolveImageSize(
			const std::vector<int64_t>& runtimeShape,
			int& width,
			int& height);

		/// <summary>
		/// ONNXモデルの先頭入力から画像サイズを取得する
		/// </summary>
		bool TryResolveModelInputSize(
			const OnnxModel* model,
			int fallbackWidth,
			int fallbackHeight,
			int& width,
			int& height);

	}

}
