#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/matrix4x4.h"
#include "engine/math/Transform.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include "engine/base/WinApp.h"
#include <json.hpp>

//============================================================================
// CameraForGPU struct
//============================================================================

//定数バッファ用の構造体
struct CameraForGPU {
	Matrix4x4 ProjectionInverse; // 投影行列の逆行列
	Vector3 worldPosition;
	float padding;
};

struct CameraConfig {

	//カメラ設定JSONファイルのパス
	std::string filePath;
	//カメラの各種パラメータ
	QuaternionTransform transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Vector3 offset_ = { 0.0f, 0.0f, -5.0f };
	Vector3 offsetDelta_ = { 0.0f, 5.0f, -55.0f };
	float fovX_ = 0.45f;
	float aspectRatio_ = float(WinApp::kScreenWidth / 2) / float(WinApp::kScreenHeight / 2);
	float nearClip_ = 0.1f;
	float farClip_ = 3800.0f;
};

// JSON変換関数
void to_json(nlohmann::json& json, const CameraConfig& cameraConfig);
void from_json(const nlohmann::json& json, CameraConfig& cameraConfig);

// JSONディレクトリパスの特殊化
template<>
struct JsonPath<CameraConfig> {
	static std::string GetDirectory() {
		return kCameraDataPath;
	}
};