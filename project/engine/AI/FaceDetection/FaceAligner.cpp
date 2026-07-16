#include "FaceAligner.h"

#include <algorithm>
#include <cmath>

#include "engine/CameraCapture/CameraCapture.h"

namespace {

	size_t CalculateElementCount(const std::vector<int64_t>& shape) {
		if (shape.empty()) {
			return 0;
		}

		size_t count = 1;
		for (int64_t dimension : shape) {
			if (dimension <= 0) {
				return 0;
			}
			count *= static_cast<size_t>(dimension);
		}
		return count;
	}

	bool ResolveImageTensorLayout(const std::vector<int64_t>& shape, int& width, int& height, int& channels, bool& isNchw) {
		if (shape.size() != 4) {
			return false;
		}

		if (shape[1] == 3 || shape[1] == 4) {
			channels = static_cast<int>(shape[1]);
			height = static_cast<int>(shape[2]);
			width = static_cast<int>(shape[3]);
			isNchw = true;
		} else if (shape[3] == 3 || shape[3] == 4) {
			height = static_cast<int>(shape[1]);
			width = static_cast<int>(shape[2]);
			channels = static_cast<int>(shape[3]);
			isNchw = false;
		} else {
			return false;
		}

		return shape[0] == 1 && width > 0 && height > 0 && channels >= 3;
	}

	size_t GetTensorIndex(bool isNchw, int width, int height, int channels, int x, int y, int channel) {
		if (isNchw) {
			return static_cast<size_t>(channel * width * height + y * width + x);
		}
		return static_cast<size_t>((y * width + x) * channels + channel);
	}

	Vector2 GetFaceLandmarkPoint(const FaceLandmark5& landmark, size_t index) {
		switch (index) {
		case 0:
			return landmark.leftEye;
		case 1:
			return landmark.rightEye;
		case 2:
			return landmark.nose;
		case 3:
			return landmark.leftMouth;
		default:
			return landmark.rightMouth;
		}
	}

}

bool FaceAligner::AlignFromCamera(
	const TakeC::CameraCapture& cameraCapture,
	const FaceDetectionResult& face,
	int detectionInputWidth,
	int detectionInputHeight,
	AlignedFaceImage& alignedImage,
	const Config& config) const {

	alignedImage = {};
	if (!cameraCapture.GetFrameRGBA() ||
		cameraCapture.GetWidth() == 0 ||
		cameraCapture.GetHeight() == 0 ||
		detectionInputWidth <= 0 ||
		detectionInputHeight <= 0 ||
		config.outputWidth <= 0 ||
		config.outputHeight <= 0) {
		return false;
	}

	const float sourceScaleX = static_cast<float>(cameraCapture.GetWidth()) / static_cast<float>(detectionInputWidth);
	const float sourceScaleY = static_cast<float>(cameraCapture.GetHeight()) / static_cast<float>(detectionInputHeight);
	Vector2 sourceLandmarks[5];
	for (size_t i = 0; i < 5; ++i) {
		const Vector2 point = GetFaceLandmarkPoint(face.landmark, i);
		sourceLandmarks[i] = { point.x * sourceScaleX, point.y * sourceScaleY };
	}

	constexpr Vector2 kTemplate112[5] = {
		{ 38.2946f, 51.6963f },
		{ 73.5318f, 51.5014f },
		{ 56.0252f, 71.7366f },
		{ 41.5493f, 92.3655f },
		{ 70.7299f, 92.2041f },
	};

	Vector2 destinationLandmarks[5];
	const float templateScaleX = static_cast<float>(config.outputWidth) / 112.0f;
	const float templateScaleY = static_cast<float>(config.outputHeight) / 112.0f;
	for (size_t i = 0; i < 5; ++i) {
		destinationLandmarks[i] = {
			kTemplate112[i].x * templateScaleX,
			kTemplate112[i].y * templateScaleY,
		};
	}

	SimilarityTransform transform;
	if (!EstimateSimilarityTransform(sourceLandmarks, destinationLandmarks, 5, transform)) {
		return false;
	}

	alignedImage.width = config.outputWidth;
	alignedImage.height = config.outputHeight;
	alignedImage.rgba.resize(static_cast<size_t>(alignedImage.width) * alignedImage.height * 4);

	const uint8_t* source = cameraCapture.GetFrameRGBA();
	for (int y = 0; y < alignedImage.height; ++y) {
		for (int x = 0; x < alignedImage.width; ++x) {
			const Vector2 destinationPoint = { static_cast<float>(x), static_cast<float>(y) };
			const Vector2 sourcePoint = ApplyInverseTransform(transform, destinationPoint);
			const size_t destinationIndex = (static_cast<size_t>(y) * alignedImage.width + x) * 4;
			alignedImage.rgba[destinationIndex + 0] = SampleRgbaBilinear(source, cameraCapture.GetWidth(), cameraCapture.GetHeight(), sourcePoint.x, sourcePoint.y, 0);
			alignedImage.rgba[destinationIndex + 1] = SampleRgbaBilinear(source, cameraCapture.GetWidth(), cameraCapture.GetHeight(), sourcePoint.x, sourcePoint.y, 1);
			alignedImage.rgba[destinationIndex + 2] = SampleRgbaBilinear(source, cameraCapture.GetWidth(), cameraCapture.GetHeight(), sourcePoint.x, sourcePoint.y, 2);
			alignedImage.rgba[destinationIndex + 3] = 255;
		}
	}

	return true;
}

bool FaceAligner::BuildInputFromAlignedFace(
	const AlignedFaceImage& alignedImage,
	std::vector<float>& inputData,
	const std::vector<int64_t>& inputShape,
	bool swapRedBlue,
	bool normalizeToUnit) const {

	if (!alignedImage.IsValid()) {
		return false;
	}

	int tensorWidth = 0;
	int tensorHeight = 0;
	int tensorChannels = 0;
	bool isNchw = true;
	if (!ResolveImageTensorLayout(inputShape, tensorWidth, tensorHeight, tensorChannels, isNchw)) {
		return false;
	}

	const size_t elementCount = CalculateElementCount(inputShape);
	if (elementCount == 0) {
		return false;
	}

	inputData.assign(elementCount, 0.0f);
	const uint32_t channelMap[3] = {
		swapRedBlue ? 2u : 0u,
		1u,
		swapRedBlue ? 0u : 2u,
	};

	const float scaleX = static_cast<float>(alignedImage.width) / static_cast<float>(tensorWidth);
	const float scaleY = static_cast<float>(alignedImage.height) / static_cast<float>(tensorHeight);
	for (int y = 0; y < tensorHeight; ++y) {
		const float sourceY = (static_cast<float>(y) + 0.5f) * scaleY - 0.5f;
		for (int x = 0; x < tensorWidth; ++x) {
			const float sourceX = (static_cast<float>(x) + 0.5f) * scaleX - 0.5f;
			for (int channel = 0; channel < 3; ++channel) {
				float value = static_cast<float>(SampleRgbaBilinear(alignedImage.rgba.data(), alignedImage.width, alignedImage.height, sourceX, sourceY, channelMap[channel]));
				if (normalizeToUnit) {
					value /= 255.0f;
				}
				inputData[GetTensorIndex(isNchw, tensorWidth, tensorHeight, tensorChannels, x, y, channel)] = value;
			}
		}
	}

	return true;
}

bool FaceAligner::EstimateSimilarityTransform(
	const Vector2* sourcePoints,
	const Vector2* destinationPoints,
	size_t pointCount,
	SimilarityTransform& transform) {

	if (!sourcePoints || !destinationPoints || pointCount < 2) {
		return false;
	}

	Vector2 sourceMean = { 0.0f, 0.0f };
	Vector2 destinationMean = { 0.0f, 0.0f };
	for (size_t i = 0; i < pointCount; ++i) {
		sourceMean.x += sourcePoints[i].x;
		sourceMean.y += sourcePoints[i].y;
		destinationMean.x += destinationPoints[i].x;
		destinationMean.y += destinationPoints[i].y;
	}
	sourceMean.x /= static_cast<float>(pointCount);
	sourceMean.y /= static_cast<float>(pointCount);
	destinationMean.x /= static_cast<float>(pointCount);
	destinationMean.y /= static_cast<float>(pointCount);

	float a = 0.0f;
	float b = 0.0f;
	float sourceVariance = 0.0f;
	for (size_t i = 0; i < pointCount; ++i) {
		const Vector2 source = { sourcePoints[i].x - sourceMean.x, sourcePoints[i].y - sourceMean.y };
		const Vector2 destination = { destinationPoints[i].x - destinationMean.x, destinationPoints[i].y - destinationMean.y };
		a += source.x * destination.x + source.y * destination.y;
		b += source.x * destination.y - source.y * destination.x;
		sourceVariance += source.x * source.x + source.y * source.y;
	}

	if (sourceVariance <= 0.00001f) {
		return false;
	}

	a /= sourceVariance;
	b /= sourceVariance;
	transform.scale = std::sqrt(a * a + b * b);
	if (transform.scale <= 0.00001f) {
		return false;
	}

	transform.cos = a / transform.scale;
	transform.sin = b / transform.scale;
	transform.translate = {
		destinationMean.x - (a * sourceMean.x - b * sourceMean.y),
		destinationMean.y - (b * sourceMean.x + a * sourceMean.y),
	};
	return true;
}

Vector2 FaceAligner::ApplyInverseTransform(const SimilarityTransform& transform, const Vector2& point) {
	const float a = transform.scale * transform.cos;
	const float b = transform.scale * transform.sin;
	const float denominator = a * a + b * b;
	if (denominator <= 0.00001f) {
		return point;
	}

	const float x = point.x - transform.translate.x;
	const float y = point.y - transform.translate.y;
	return {
		(a * x + b * y) / denominator,
		(-b * x + a * y) / denominator,
	};
}

uint8_t FaceAligner::SampleRgbaBilinear(const uint8_t* source, int width, int height, float x, float y, int channel) {
	if (!source || width <= 0 || height <= 0 || channel < 0 || channel >= 4) {
		return 0;
	}

	x = std::clamp(x, 0.0f, static_cast<float>(width - 1));
	y = std::clamp(y, 0.0f, static_cast<float>(height - 1));

	const int x0 = static_cast<int>(std::floor(x));
	const int y0 = static_cast<int>(std::floor(y));
	const int x1 = std::min(x0 + 1, width - 1);
	const int y1 = std::min(y0 + 1, height - 1);
	const float tx = x - static_cast<float>(x0);
	const float ty = y - static_cast<float>(y0);

	const auto at = [&](int sampleX, int sampleY) {
		return static_cast<float>(source[(static_cast<size_t>(sampleY) * width + sampleX) * 4 + channel]);
	};

	const float top = at(x0, y0) * (1.0f - tx) + at(x1, y0) * tx;
	const float bottom = at(x0, y1) * (1.0f - tx) + at(x1, y1) * tx;
	return static_cast<uint8_t>(std::clamp(top * (1.0f - ty) + bottom * ty, 0.0f, 255.0f));
}
