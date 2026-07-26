#include "Landmark106Decoder.h"

std::vector<Vector2> TakeC::Landmark106Decoder::Decode(
	const std::vector<std::vector<float>>& outputDataList,
	int alignedWidth,
	int alignedHeight,
	const Config& config) {

	std::vector<Vector2> points;
	const size_t requiredElementCount = kLandmarkCount * 2;
	if (outputDataList.empty() ||
		outputDataList[0].size() < requiredElementCount ||
		alignedWidth <= 0 ||
		alignedHeight <= 0) {
		return points;
	}

	const std::vector<float>& output = outputDataList[0];
	points.reserve(kLandmarkCount);
	for (size_t i = 0; i < kLandmarkCount; ++i) {
		float x = output[i * 2 + 0];
		float y = output[i * 2 + 1];

		if (config.outputMinusOneToOne) {
			x = (x + 1.0f) * 0.5f * static_cast<float>(alignedWidth);
			y = (y + 1.0f) * 0.5f * static_cast<float>(alignedHeight);
		} else if (x >= 0.0f && x <= 1.5f && y >= 0.0f && y <= 1.5f) {
			x *= static_cast<float>(alignedWidth);
			y *= static_cast<float>(alignedHeight);
		}

		points.push_back({ x, y });
	}

	return points;
}
