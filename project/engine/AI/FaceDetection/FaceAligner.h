#pragma once

#include "FaceDetectionContext.h"

#include <cstdint>
#include <vector>

namespace TakeC {
	class CameraCapture;
}

/// <summary>
/// AlignedFaceImageに必要な値をまとめて保持する構造体です。
/// </summary>
struct AlignedFaceImage {
	int width = 0;
	int height = 0;
	std::vector<uint8_t> rgba;

	bool IsValid() const { return width > 0 && height > 0 && !rgba.empty(); }
};

/// <summary>
/// 顔矩形と5点ランドマークから推論用の位置合わせ済み顔画像を生成するクラスです。
/// </summary>
class FaceAligner {
public:
	/// <summary>
	/// Configで使用する設定値を保持する構造体です。
	/// </summary>
	struct Config {
		int outputWidth = 112;
		int outputHeight = 112;
	};

	bool AlignFromCamera(
		const TakeC::CameraCapture& cameraCapture,
		const FaceDetectionResult& face,
		int detectionInputWidth,
		int detectionInputHeight,
		AlignedFaceImage& alignedImage,
		const Config& config = {}) const;

	bool BuildInputFromAlignedFace(
		const AlignedFaceImage& alignedImage,
		std::vector<float>& inputData,
		const std::vector<int64_t>& inputShape,
		bool swapRedBlue = false,
		bool normalizeToUnit = true) const;

private:
	/// <summary>
	/// SimilarityTransformの数学的な値と変換情報を表す構造体です。
	/// </summary>
	struct SimilarityTransform {
		float scale = 1.0f;
		float cos = 1.0f;
		float sin = 0.0f;
		Vector2 translate = { 0.0f, 0.0f };
	};

	static bool EstimateSimilarityTransform(
		const Vector2* sourcePoints,
		const Vector2* destinationPoints,
		size_t pointCount,
		SimilarityTransform& transform);

	static Vector2 ApplyInverseTransform(const SimilarityTransform& transform, const Vector2& point);
	static uint8_t SampleRgbaBilinear(const uint8_t* source, int width, int height, float x, float y, int channel);
};
