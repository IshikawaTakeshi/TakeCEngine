#include "FaceInputBuilder.h"

#include <algorithm>
#include <cmath>

#include "engine/CameraCapture/CameraCapture.h"

namespace {

	struct TensorLayout {
		int64_t batch = 1;
		int64_t channels = 0;
		int64_t height = 0;
		int64_t width = 0;
		bool isNchw = true;
	};

	size_t CalculateElementCount(const std::vector<int64_t>& shape) {
		if (shape.empty()) {
			return 0;
		}

		size_t elementCount = 1;
		for (int64_t dimension : shape) {
			if (dimension <= 0) {
				return 0;
			}
			elementCount *= static_cast<size_t>(dimension);
		}
		return elementCount;
	}

	bool ResolveImageTensorLayout(const std::vector<int64_t>& inputShape, TensorLayout& layout) {
		if (inputShape.size() != 4) {
			return false;
		}

		if (inputShape[1] == 3 || inputShape[1] == 4) {
			layout.batch = inputShape[0];
			layout.channels = inputShape[1];
			layout.height = inputShape[2];
			layout.width = inputShape[3];
			layout.isNchw = true;
		} else if (inputShape[3] == 3 || inputShape[3] == 4) {
			layout.batch = inputShape[0];
			layout.height = inputShape[1];
			layout.width = inputShape[2];
			layout.channels = inputShape[3];
			layout.isNchw = false;
		} else {
			return false;
		}

		return layout.batch == 1 && layout.channels >= 3 && layout.height > 0 && layout.width > 0;
	}

	float SampleChannelBilinear(
		const uint8_t* source,
		uint32_t sourceWidth,
		uint32_t sourceHeight,
		float sourceX,
		float sourceY,
		uint32_t channel) {

		sourceX = std::clamp(sourceX, 0.0f, static_cast<float>(sourceWidth - 1));
		sourceY = std::clamp(sourceY, 0.0f, static_cast<float>(sourceHeight - 1));

		const uint32_t x0 = static_cast<uint32_t>(std::floor(sourceX));
		const uint32_t y0 = static_cast<uint32_t>(std::floor(sourceY));
		const uint32_t x1 = std::min(x0 + 1, sourceWidth - 1);
		const uint32_t y1 = std::min(y0 + 1, sourceHeight - 1);
		const float tx = sourceX - static_cast<float>(x0);
		const float ty = sourceY - static_cast<float>(y0);

		const auto at = [&](uint32_t x, uint32_t y) -> float {
			return static_cast<float>(source[(static_cast<size_t>(y) * sourceWidth + x) * 4 + channel]);
			};

		const float top = at(x0, y0) * (1.0f - tx) + at(x1, y0) * tx;
		const float bottom = at(x0, y1) * (1.0f - tx) + at(x1, y1) * tx;
		return top * (1.0f - ty) + bottom * ty;
	}

	size_t GetTensorIndex(const TensorLayout& layout, int64_t y, int64_t x, int64_t channel) {
		if (layout.isNchw) {
			return static_cast<size_t>(channel * layout.height * layout.width + y * layout.width + x);
		}
		return static_cast<size_t>((y * layout.width + x) * layout.channels + channel);
	}

}

bool TakeC::BuildFaceInputFromCamera(
	const CameraCapture& cameraCapture,
	std::vector<float>& inputData,
	const std::vector<int64_t>& inputShape,
	const FaceInputBuildSettings& settings) {

	const size_t inputElementCount = CalculateElementCount(inputShape);
	if (inputElementCount == 0) {
		return false;
	}

	inputData.resize(inputElementCount);
	return BuildFaceInputFromCamera(cameraCapture, inputData.data(), inputData.size(), inputShape, settings);
}

bool TakeC::BuildFaceInputFromCamera(
	const CameraCapture& cameraCapture,
	float* inputData,
	size_t inputElementCount,
	const std::vector<int64_t>& inputShape,
	const FaceInputBuildSettings& settings) {

	if (!inputData || !cameraCapture.GetFrameRGBA() || cameraCapture.GetWidth() == 0 || cameraCapture.GetHeight() == 0) {
		return false;
	}

	TensorLayout layout;
	if (!ResolveImageTensorLayout(inputShape, layout)) {
		return false;
	}

	if (inputElementCount < CalculateElementCount(inputShape)) {
		return false;
	}

	const uint8_t* source = cameraCapture.GetFrameRGBA();
	const uint32_t sourceWidth = cameraCapture.GetWidth();
	const uint32_t sourceHeight = cameraCapture.GetHeight();
	const float scaleX = static_cast<float>(sourceWidth) / static_cast<float>(layout.width);
	const float scaleY = static_cast<float>(sourceHeight) / static_cast<float>(layout.height);
	const uint32_t channelMap[3] = {
		settings.swapRedBlue ? 2u : 0u,
		1u,
		settings.swapRedBlue ? 0u : 2u,
	};

	std::fill(inputData, inputData + inputElementCount, 0.0f);

	for (int64_t y = 0; y < layout.height; ++y) {
		const float sourceY = (static_cast<float>(y) + 0.5f) * scaleY - 0.5f;
		for (int64_t x = 0; x < layout.width; ++x) {
			const float sourceX = (static_cast<float>(x) + 0.5f) * scaleX - 0.5f;
			for (int64_t channel = 0; channel < 3; ++channel) {
				float value = SampleChannelBilinear(source, sourceWidth, sourceHeight, sourceX, sourceY, channelMap[channel]);
				if (settings.normalizeToUnit) {
					value /= 255.0f;
				}
				value = (value - settings.mean[channel]) / settings.standardDeviation[channel];
				inputData[GetTensorIndex(layout, y, x, channel)] = value;
			}
		}
	}

	return true;
}
