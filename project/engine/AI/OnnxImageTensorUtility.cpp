#include "OnnxImageTensorUtility.h"

#include "engine/AI/OnnxModel.h"

namespace {

	constexpr int kDefaultInputWidth = 640;
	constexpr int kDefaultInputHeight = 640;

}

bool TakeC::OnnxImageTensorUtility::ResolveRuntimeInputShape(
	const std::vector<int64_t>& modelShape,
	std::vector<int64_t>& runtimeShape) {

	return ResolveRuntimeInputShape(
		modelShape,
		kDefaultInputWidth,
		kDefaultInputHeight,
		runtimeShape);
}

bool TakeC::OnnxImageTensorUtility::ResolveRuntimeInputShape(
	const std::vector<int64_t>& modelShape,
	int fallbackWidth,
	int fallbackHeight,
	std::vector<int64_t>& runtimeShape) {

	if (modelShape.size() != 4 || fallbackWidth <= 0 || fallbackHeight <= 0) {
		return false;
	}

	// modelShapeはONNXモデルに定義されている情報。
	// dynamic次元の場合は、推論実行用shapeへ具体的な値を設定する。
	runtimeShape = modelShape;
	if (runtimeShape[0] <= 0) {
		runtimeShape[0] = 1;
	}

	const bool mayBeNchw =
		runtimeShape[1] == 3 ||
		runtimeShape[1] == 4 ||
		runtimeShape[1] <= 0;
	const bool mayBeNhwc =
		runtimeShape[3] == 3 ||
		runtimeShape[3] == 4 ||
		runtimeShape[3] <= 0;

	if (mayBeNchw) {
		if (runtimeShape[1] <= 0) {
			runtimeShape[1] = 3;
		}
		if (runtimeShape[2] <= 0) {
			runtimeShape[2] = fallbackHeight;
		}
		if (runtimeShape[3] <= 0) {
			runtimeShape[3] = fallbackWidth;
		}
	} else if (mayBeNhwc) {
		if (runtimeShape[1] <= 0) {
			runtimeShape[1] = fallbackHeight;
		}
		if (runtimeShape[2] <= 0) {
			runtimeShape[2] = fallbackWidth;
		}
		if (runtimeShape[3] <= 0) {
			runtimeShape[3] = 3;
		}
	} else {
		return false;
	}

	return runtimeShape[0] == 1;
}

bool TakeC::OnnxImageTensorUtility::ResolveImageSize(
	const std::vector<int64_t>& runtimeShape,
	int& width,
	int& height) {

	if (runtimeShape.size() != 4) {
		return false;
	}

	if (runtimeShape[1] == 3 || runtimeShape[1] == 4) {
		height = static_cast<int>(runtimeShape[2]);
		width = static_cast<int>(runtimeShape[3]);
		return width > 0 && height > 0;
	}

	if (runtimeShape[3] == 3 || runtimeShape[3] == 4) {
		height = static_cast<int>(runtimeShape[1]);
		width = static_cast<int>(runtimeShape[2]);
		return width > 0 && height > 0;
	}

	return false;
}

bool TakeC::OnnxImageTensorUtility::TryResolveModelInputSize(
	const OnnxModel* model,
	int fallbackWidth,
	int fallbackHeight,
	int& width,
	int& height) {

	if (!model || model->GetInputShapes().empty()) {
		return false;
	}

	std::vector<int64_t> runtimeShape;
	if (!ResolveRuntimeInputShape(
		model->GetInputShapes()[0],
		fallbackWidth,
		fallbackHeight,
		runtimeShape)) {
		return false;
	}

	return ResolveImageSize(runtimeShape, width, height);
}
