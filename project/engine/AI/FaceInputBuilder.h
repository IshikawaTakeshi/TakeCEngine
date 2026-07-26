#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace TakeC {

	class CameraCapture;

	/// <summary>
	/// FaceInputBuildSettingsで使用する設定値を保持する構造体です。
	/// </summary>
	struct FaceInputBuildSettings {
		bool swapRedBlue = false;
		bool normalizeToUnit = true;
		float mean[3] = { 0.0f, 0.0f, 0.0f };
		float standardDeviation[3] = { 1.0f, 1.0f, 1.0f };
	};

	bool BuildFaceInputFromCamera(
		const CameraCapture& cameraCapture,
		std::vector<float>& inputData,
		const std::vector<int64_t>& inputShape,
		const FaceInputBuildSettings& settings = {});

	bool BuildFaceInputFromCamera(
		const CameraCapture& cameraCapture,
		float* inputData,
		size_t inputElementCount,
		const std::vector<int64_t>& inputShape,
		const FaceInputBuildSettings& settings = {});

}
