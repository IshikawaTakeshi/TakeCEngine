#include "TitleScene.h"
#include <algorithm>
#include <vector>

#include "engine/Scene/SceneManager.h"
#include "application/Tool/FaceAnalysisDebugRenderer.h"

#include "engine/AI/FaceAnalysis/FacePartRatioCalculator.h"
#include "engine/AI/FaceDetection/Landmark106Decoder.h"
#include "engine/AI/FaceInputBuilder.h"
#include "engine/AI/OnnxImageTensorUtility.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/CameraCapture/CameraCapture.h"
#include "engine/Math/Quaternion.h"
#include "engine/Utility/StringUtility.h"
#include "externals/imgui/imgui.h"

using namespace TakeC;

//====================================================================
//			初期化
//====================================================================
void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_unique<Camera>();
	camera0_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(), "Title_ViewCamera.json");
	camera0_->SetRotationSpeed(1.0f);
	camera0_->SetFollowSpeed(1.0f);
	TakeC::CameraManager::GetInstance().AddCamera("Tcamera0", camera0_.get());

	//デフォルトカメラの設定
	Object3dCommon::GetInstance().SetDefaultCamera(TakeC::CameraManager::GetInstance().GetActiveCamera());

	// SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(),
		"skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f, 0.2f, 0.2f, 1.0f });

	// ShadowMapEffectを無効化
	TakeC::TakeCFrameWork::GetPostEffectManager()->SetEffectActive("ShadowMapEffect", false);
	TakeC::TakeCFrameWork::GetPostEffectManager()->SetEffectActive("DepthBasedOutline", false);

	debugOnnxModel_ = TakeC::TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
		"TitleSceneDebugModel",
		StringUtility::ConvertString(debugOnnxModelPath_.data()));
	debugOnnxLoadFailed_ = debugOnnxModel_ == nullptr;

	debugLandmarkModel_ = TakeC::TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
		"TitleSceneLandmarkModel",
		StringUtility::ConvertString(debugLandmarkModelPath_.data()));
	debugLandmarkLoadFailed_ = debugLandmarkModel_ == nullptr;
	if (int landmarkInputWidth = 0, landmarkInputHeight = 0;
		OnnxImageTensorUtility::TryResolveModelInputSize(
			debugLandmarkModel_,
			192,
			192,
			landmarkInputWidth,
			landmarkInputHeight)) {
		debugAlignedFaceSize_ = std::max(landmarkInputWidth, landmarkInputHeight);
	}
}

//====================================================================
//			終了処理
//====================================================================
void TitleScene::Finalize() {
	skyBox_.reset();
	camera0_.reset();
	camera1_.reset();
	TakeC::CameraManager::GetInstance().ResetCameras();
	TakeC::TakeCFrameWork::GetSpriteManager()->Clear();
}

//====================================================================
//			更新処理
//====================================================================
void TitleScene::Update() {

	//CameraCaptureの更新
	TakeC::CameraCapture* cameraCapture = TakeC::TakeCFrameWork::GetCameraCapture();
	if (cameraCapture) {
		cameraCapture->Update();
	}

	// ONNXモデルの更新。
	// GetInputShapes()はモデル定義の情報、runtimeInputShapeは実際の推論に使う具体shape。
	if (debugOnnxInferenceTimer_ > 0.0f) {
		debugOnnxInferenceTimer_ -= TakeC::TakeCFrameWork::GetDeltaTime();
	}

	if (debugOnnxInferenceEnabled_ && debugOnnxInferenceTimer_ <= 0.0f &&
		debugOnnxModel_ && cameraCapture && cameraCapture->GetFrameRGBA()) {

		debugOnnxRunSuccess_ = false;
		const auto& modelInputShapes = debugOnnxModel_->GetInputShapes();
		if (!modelInputShapes.empty()) {
			std::vector<int64_t> runtimeInputShape;
			if (OnnxImageTensorUtility::ResolveRuntimeInputShape(modelInputShapes[0], runtimeInputShape) &&
				debugOnnxModel_->ResizeInputBuffer(0, runtimeInputShape)) {

				std::vector<float>* input = debugOnnxModel_->GetInputData(0);

				// カメラキャプチャから入力テンソルを構築
				if (input && TakeC::BuildFaceInputFromCamera(*cameraCapture, *input, runtimeInputShape)) {

					// ONNXモデルの実行
					debugOnnxRunSuccess_ = debugOnnxModel_->Run();

					// 推論成功時に顔検出結果をデコード
					if (debugOnnxRunSuccess_) {
						int inputWidth = 0;
						int inputHeight = 0;

						// runtimeInputShapeから画像サイズを取得
						if (OnnxImageTensorUtility::ResolveImageSize(runtimeInputShape, inputWidth, inputHeight)) {

							// 顔検出結果のデコード設定
							ScrfdDecoder::Config config;
							config.inputWidth = inputWidth;
							config.inputHeight = inputHeight;
							config.scoreThreshold = debugScrfdScoreThreshold_;
							config.nmsThreshold = debugScrfdNmsThreshold_;
							config.multiplyByStride = debugScrfdMultiplyByStride_;
							config.anchorCenterOffset = debugScrfdAnchorCenterOffset_;
							config.decodeOffsetX = debugScrfdDecodeOffsetX_;
							config.decodeOffsetY = debugScrfdDecodeOffsetY_;

							// 顔検出結果のデコード
							debugFaceResults_ = debugScrfdDecoder_.Decode(
								debugOnnxModel_->GetOutputNames(),
								debugOnnxModel_->GetOutputDataList(),
								debugOnnxModel_->GetRuntimeOutputShapes(),
								config);
							// 顔検出結果を入力サイズに設定
							debugScrfdInputWidth_ = inputWidth;
							debugScrfdInputHeight_ = inputHeight;

							debugFaceAlignSuccess_ = false;
							debugAlignedFaceImage_ = {};

							// 顔検出結果が存在する場合、最初の顔をアラインメント
							if (!debugFaceResults_.empty()) {
								FaceAligner::Config alignConfig;
								alignConfig.outputWidth = debugAlignedFaceSize_;
								alignConfig.outputHeight = debugAlignedFaceSize_;

								// 顔アラインメントの実行
								debugFaceAlignSuccess_ = debugFaceAligner_.AlignFromCamera(
									*cameraCapture,
									debugFaceResults_.front(),
									debugScrfdInputWidth_,
									debugScrfdInputHeight_,
									debugAlignedFaceImage_,
									alignConfig);
							}

							debugLandmarkRunSuccess_ = false;
							debugLandmark106Points_.clear();
							debugFacePartRatios_.reset();

							// 顔アラインメントが成功し、ランドマークモデルがロードされている場合、ランドマーク推論を実行
							if (debugFaceAlignSuccess_ && debugLandmarkModel_) {
								const auto& landmarkModelInputShapes = debugLandmarkModel_->GetInputShapes();
								if (!landmarkModelInputShapes.empty()) {
									std::vector<int64_t> landmarkRuntimeInputShape;
									if (OnnxImageTensorUtility::ResolveRuntimeInputShape(
										landmarkModelInputShapes[0],
										debugAlignedFaceImage_.width,
										debugAlignedFaceImage_.height,
										landmarkRuntimeInputShape) &&
										debugLandmarkModel_->ResizeInputBuffer(0, landmarkRuntimeInputShape)) {

										std::vector<float>* landmarkInput = debugLandmarkModel_->GetInputData(0);
										if (landmarkInput &&
											debugFaceAligner_.BuildInputFromAlignedFace(
												debugAlignedFaceImage_,
												*landmarkInput,
												landmarkRuntimeInputShape,
												debugLandmarkSwapRedBlue_,
												debugLandmarkNormalizeToUnit_)) {

											debugLandmarkRunSuccess_ = debugLandmarkModel_->Run();
											if (debugLandmarkRunSuccess_) {
												Landmark106Decoder::Config landmarkConfig;
												landmarkConfig.outputMinusOneToOne = debugLandmarkOutputMinusOneToOne_;
												debugLandmark106Points_ = Landmark106Decoder::Decode(
													debugLandmarkModel_->GetOutputDataList(),
													debugAlignedFaceImage_.width,
													debugAlignedFaceImage_.height,
													landmarkConfig);
												debugFacePartRatios_ = FacePartRatioCalculator::Calculate(
													debugLandmark106Points_);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		debugOnnxInferenceTimer_ = std::max(debugOnnxInferenceInterval_, 0.0f);
	}


	//カメラの更新
	TakeC::CameraManager::GetInstance().Update();

	//SkyBoxの更新
	skyBox_->Update();

	//シーン遷移
	if (TakeC::Input::GetInstance().TriggerButton(0, GamepadButtonType::A)) {
		//シーン切り替え依頼
		//EnemySelectSceneへ
		SceneManager::GetInstance().ChangeScene("GAMEPLAY", 1.0f);
	}
}

//====================================================================
//			ImGui更新処理
//====================================================================
void TitleScene::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	//ImGuiの更新
	TakeC::CameraManager::GetInstance().UpdateImGui();
	TakeC::TakeCFrameWork::GetSpriteManager()->UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();
	if (TakeC::CameraCapture* cameraCapture = TakeC::TakeCFrameWork::GetCameraCapture()) {
		cameraCapture->UpdateImGui();
		FaceAnalysisDebugRenderer::DrawFaceDetectionOverlay(
			*cameraCapture,
			debugFaceResults_,
			debugScrfdInputWidth_,
			debugScrfdInputHeight_);
	}

	ImGui::Begin("ONNX Model Debug");
	ImGui::InputText("Model Path", debugOnnxModelPath_.data(), debugOnnxModelPath_.size());
	if (ImGui::Button("Load ONNX Model")) {
		debugOnnxModel_ = TakeC::TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
			"TitleSceneDebugModel",
			StringUtility::ConvertString(debugOnnxModelPath_.data()));
		debugOnnxLoadFailed_ = debugOnnxModel_ == nullptr;
	}
	ImGui::InputText("Landmark Model Path", debugLandmarkModelPath_.data(), debugLandmarkModelPath_.size());
	if (ImGui::Button("Load Landmark Model")) {
		debugLandmarkModel_ = TakeC::TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
			"TitleSceneLandmarkModel",
			StringUtility::ConvertString(debugLandmarkModelPath_.data()));
		debugLandmarkLoadFailed_ = debugLandmarkModel_ == nullptr;
		if (int landmarkInputWidth = 0, landmarkInputHeight = 0;
			OnnxImageTensorUtility::TryResolveModelInputSize(
				debugLandmarkModel_,
				192,
				192,
				landmarkInputWidth,
				landmarkInputHeight)) {
			debugAlignedFaceSize_ = std::max(landmarkInputWidth, landmarkInputHeight);
		}
	}

	if (debugOnnxModel_) {
		ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Loaded");
		ImGui::Checkbox("Enable Inference", &debugOnnxInferenceEnabled_);
		ImGui::SliderFloat("Inference Interval", &debugOnnxInferenceInterval_, 0.0f, 1.0f, "%.3f sec");
		ImGui::SliderFloat("SCRFD Score Threshold", &debugScrfdScoreThreshold_, 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("SCRFD NMS Threshold", &debugScrfdNmsThreshold_, 0.0f, 1.0f, "%.2f");
		ImGui::Checkbox("SCRFD Multiply By Stride", &debugScrfdMultiplyByStride_);
		ImGui::SliderFloat("SCRFD Anchor Center Offset", &debugScrfdAnchorCenterOffset_, 0.0f, 0.5f, "%.2f");
		ImGui::SliderFloat("SCRFD Decode Offset X", &debugScrfdDecodeOffsetX_, -64.0f, 64.0f, "%.1f px");
		ImGui::SliderFloat("SCRFD Decode Offset Y", &debugScrfdDecodeOffsetY_, -64.0f, 64.0f, "%.1f px");
		ImGui::SliderInt("Aligned Face Size", &debugAlignedFaceSize_, 64, 256);
		ImGui::Checkbox("Landmark Output -1..1", &debugLandmarkOutputMinusOneToOne_);
		ImGui::Checkbox("Landmark Swap R/B", &debugLandmarkSwapRedBlue_);
		ImGui::Checkbox("Landmark Normalize 0..1", &debugLandmarkNormalizeToUnit_);

		if (debugOnnxRunSuccess_) {
			ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Run successful.");
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Run failed.");
		}

		FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
			"Model Inputs",
			debugOnnxModel_->GetInputNames(),
			debugOnnxModel_->GetInputShapes());
		FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
			"Model Outputs",
			debugOnnxModel_->GetOutputNames(),
			debugOnnxModel_->GetOutputShapes());
		FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
			"Runtime Inputs",
			debugOnnxModel_->GetInputNames(),
			debugOnnxModel_->GetRuntimeInputShapes());
		FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
			"Runtime Outputs",
			debugOnnxModel_->GetOutputNames(),
			debugOnnxModel_->GetRuntimeOutputShapes());

		if (debugLandmarkModel_) {
			ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Landmark model loaded.");
			ImGui::Text("Landmark Run: %s", debugLandmarkRunSuccess_ ? "success" : "not run / failed");
			FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
				"Landmark Model Inputs",
				debugLandmarkModel_->GetInputNames(),
				debugLandmarkModel_->GetInputShapes());
			FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
				"Landmark Model Outputs",
				debugLandmarkModel_->GetOutputNames(),
				debugLandmarkModel_->GetOutputShapes());
			FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
				"Landmark Runtime Inputs",
				debugLandmarkModel_->GetInputNames(),
				debugLandmarkModel_->GetRuntimeInputShapes());
			FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
				"Landmark Runtime Outputs",
				debugLandmarkModel_->GetOutputNames(),
				debugLandmarkModel_->GetRuntimeOutputShapes());
		} else if (debugLandmarkLoadFailed_) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Landmark model load failed.");
		}

		ImGui::Text("Detected faces: %d", static_cast<int>(debugFaceResults_.size()));
		if (debugFaceAlignSuccess_ && debugAlignedFaceImage_.IsValid()) {
			ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f),
				"Aligned face generated: %d x %d",
				debugAlignedFaceImage_.width,
				debugAlignedFaceImage_.height);
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Aligned face not generated.");
		}
		ImGui::Text("106 landmarks: %d", static_cast<int>(debugLandmark106Points_.size()));
		if (!debugLandmark106Points_.empty()) {
			ImGui::Text("landmark[0]: %.1f, %.1f", debugLandmark106Points_[0].x, debugLandmark106Points_[0].y);
			ImGui::Text("landmark[105]: %.1f, %.1f", debugLandmark106Points_[105].x, debugLandmark106Points_[105].y);
			FaceAnalysisDebugRenderer::DrawLandmark106Canvas(
				debugLandmark106Points_,
				debugAlignedFaceImage_.width,
				debugAlignedFaceImage_.height);
		}
		if (debugFacePartRatios_) {
			FaceAnalysisDebugRenderer::DrawFacePartRatios(*debugFacePartRatios_);
		}
		const size_t displayCount = std::min<size_t>(debugFaceResults_.size(), 5);
		for (size_t i = 0; i < displayCount; ++i) {
			FaceAnalysisDebugRenderer::DrawFaceDetectionResult(debugFaceResults_[i], i);
		}
	}
	else if (debugOnnxLoadFailed_) {
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Load failed. Check model path and Output window log.");
	}
	else {
		ImGui::TextUnformatted("No model loaded.");
	}
	ImGui::End();

#endif
}

//====================================================================
//			描画処理
//====================================================================
void TitleScene::Draw() {

	//SkyBox描画
	skyBox_->Draw();

	Object3dCommon::GetInstance().Dispatch();

	Object3dCommon::GetInstance().PreDraw();
}

void TitleScene::DrawSprite() {
	//タイトルテキスト描画
	TakeC::SpriteCommon::GetInstance().PreDraw();
}

void TitleScene::DrawShadow() {
}
