#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "engine/AI/FaceDetection/FaceDetectionContext.h"
#include "engine/Math/Vector2.h"

namespace TakeC {

	class CameraCapture;
	struct FacePartRatios;

	//==============================================================================================
	//		FaceAnalysisDebugRenderer class
	//==============================================================================================
	class FaceAnalysisDebugRenderer {
	public:

		static void DrawOnnxTensorInfo(
			const char* label,
			const std::vector<std::string>& names,
			const std::vector<std::vector<int64_t>>& shapes);

		static void DrawFaceDetectionResult(
			const FaceDetectionResult& result,
			size_t index);

		static void DrawFaceDetectionOverlay(
			const CameraCapture& cameraCapture,
			const std::vector<FaceDetectionResult>& results,
			int inputWidth,
			int inputHeight);

		static void DrawLandmark106Canvas(
			const std::vector<Vector2>& points,
			int width,
			int height);

		static void DrawFacePartRatios(const FacePartRatios& ratios);

	private:

		static std::string ShapeToString(const std::vector<int64_t>& shape);

	};

}
