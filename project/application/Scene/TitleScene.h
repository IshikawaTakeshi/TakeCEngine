#pragma once
#include <array>
#include <optional>
#include <vector>
//base class
#include "application/Scene/BaseScene.h"

//engine
#include "engine/AI/FaceAnalysis/FacePartRatios.h"
#include "engine/AI/FaceDetection/FaceAligner.h"
#include "engine/AI/FaceDetection/ScrfdDecoder.h"
#include "engine/audio/Audio.h"
#include "engine/camera/Camera.h"
#include "engine/camera/CameraManager.h"
#include "engine/Input/Input.h"
#include "engine/base/ModelManager.h"
#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/SkyBox/SkyBox.h"

//application
#include "application/UI/PushStartUI.h"

namespace TakeC {
	class OnnxModel;
}

//============================================================================
// TitleScene class
//============================================================================
class TitleScene : public BaseScene {
public:

	//初期化
	void Initialize() override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update() override;

	//ImGuiの更新処理
	void UpdateImGui() override;

	//描画処理
	void Draw() override;
	//スプライト描画処理
	void DrawSprite() override;
	//シャドウ描画処理
	void DrawShadow() override;

private:

	//サウンドデータ
	//AudioManager::SoundData soundData1;
	// カメラ
	std::unique_ptr<Camera> camera0_ = nullptr;
	std::unique_ptr<Camera> camera1_ = nullptr;
	//SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;
	//スプライト
	TakeC::Sprite* titleTextSprite_ = nullptr;
	//ONNX debug
	TakeC::OnnxModel* debugOnnxModel_ = nullptr;
	std::array<char, 260> debugOnnxModelPath_ = { "OnnxModels/scrfd_10g_bnkps.onnx" };
	TakeC::OnnxModel* debugLandmarkModel_ = nullptr;
	std::array<char, 260> debugLandmarkModelPath_ = { "OnnxModels/2d106det.onnx" };
	bool debugOnnxLoadFailed_ = false;
	bool debugOnnxRunSuccess_ = false;
	bool debugLandmarkLoadFailed_ = false;
	bool debugLandmarkRunSuccess_ = false;
	bool debugOnnxInferenceEnabled_ = true;
	float debugOnnxInferenceInterval_ = 0.5f;
	float debugOnnxInferenceTimer_ = 0.0f;
	float debugScrfdScoreThreshold_ = 0.5f;
	float debugScrfdNmsThreshold_ = 0.4f;
	bool debugScrfdMultiplyByStride_ = true;
	float debugScrfdAnchorCenterOffset_ = 0.0f;
	float debugScrfdDecodeOffsetX_ = 0.0f;
	float debugScrfdDecodeOffsetY_ = 0.0f;
	int debugScrfdInputWidth_ = 640;
	int debugScrfdInputHeight_ = 640;
	ScrfdDecoder debugScrfdDecoder_;
	std::vector<FaceDetectionResult> debugFaceResults_;
	FaceAligner debugFaceAligner_;
	AlignedFaceImage debugAlignedFaceImage_;
	bool debugFaceAlignSuccess_ = false;
	int debugAlignedFaceSize_ = 112;
	bool debugLandmarkOutputMinusOneToOne_ = true;
	bool debugLandmarkSwapRedBlue_ = false;
	bool debugLandmarkNormalizeToUnit_ = true;
	std::vector<Vector2> debugLandmark106Points_;
	std::optional<TakeC::FacePartRatios> debugFacePartRatios_;

	std::vector<float> outputData;
	std::vector<int64_t> outputShape;
};
