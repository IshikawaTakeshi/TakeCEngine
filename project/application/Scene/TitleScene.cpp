#include "TitleScene.h"
#include <algorithm>
#include <string>
#include <vector>

#include "application/Scene/SceneManager.h"

#include "engine/AI/FaceInputBuilder.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/CameraCapture/CameraCapture.h"
#include "engine/Math/Quaternion.h"
#include "engine/Utility/StringUtility.h"
#include "externals/imgui/imgui.h"

using namespace TakeC;

namespace {
	std::string ShapeToString(const std::vector<int64_t>& shape) {
		if (shape.empty()) {
			return "[]";
		}

		std::string result = "[";
		for (size_t i = 0; i < shape.size(); ++i) {
			if (i != 0) {
				result += ", ";
			}
			result += shape[i] < 0 ? "dynamic" : std::to_string(shape[i]);
		}
		result += "]";
		return result;
	}

	void DrawOnnxTensorInfo(
		const char* label,
		const std::vector<std::string>& names,
		const std::vector<std::vector<int64_t>>& shapes) {

		if (!ImGui::TreeNode(label)) {
			return;
		}

		for (size_t i = 0; i < names.size(); ++i) {
			const std::string shape = i < shapes.size() ? ShapeToString(shapes[i]) : "[]";
			ImGui::Text("Name : %s", names[i].c_str());
			ImGui::Text("Shape: %s", shape.c_str());
			ImGui::Separator();
		}

		ImGui::TreePop();
	}

	bool ResolveRuntimeImageInputShape(const std::vector<int64_t>& modelShape, std::vector<int64_t>& runtimeShape) {
		constexpr int64_t kDefaultInputWidth = 640;
		constexpr int64_t kDefaultInputHeight = 640;

		if (modelShape.size() != 4) {
			return false;
		}

		// modelShapeはONNXモデルに定義されている情報。dynamic(-1)の場合は推論実行用に具体値を入れる。
		runtimeShape = modelShape;
		if (runtimeShape[0] <= 0) {
			runtimeShape[0] = 1;
		}

		const bool mayBeNchw = runtimeShape[1] == 3 || runtimeShape[1] == 4 || runtimeShape[1] <= 0;
		const bool mayBeNhwc = runtimeShape[3] == 3 || runtimeShape[3] == 4 || runtimeShape[3] <= 0;

		if (mayBeNchw) {
			if (runtimeShape[1] <= 0) {
				runtimeShape[1] = 3;
			}
			if (runtimeShape[2] <= 0) {
				runtimeShape[2] = kDefaultInputHeight;
			}
			if (runtimeShape[3] <= 0) {
				runtimeShape[3] = kDefaultInputWidth;
			}
		}
		else if (mayBeNhwc) {
			if (runtimeShape[1] <= 0) {
				runtimeShape[1] = kDefaultInputHeight;
			}
			if (runtimeShape[2] <= 0) {
				runtimeShape[2] = kDefaultInputWidth;
			}
			if (runtimeShape[3] <= 0) {
				runtimeShape[3] = 3;
			}
		}
		else {
			return false;
		}

		return runtimeShape[0] == 1;
	}

	bool ResolveRuntimeImageInputShapeWithFallback(
		const std::vector<int64_t>& modelShape,
		int fallbackWidth,
		int fallbackHeight,
		std::vector<int64_t>& runtimeShape) {

		if (modelShape.size() != 4 || fallbackWidth <= 0 || fallbackHeight <= 0) {
			return false;
		}

		runtimeShape = modelShape;
		if (runtimeShape[0] <= 0) {
			runtimeShape[0] = 1;
		}

		const bool mayBeNchw = runtimeShape[1] == 3 || runtimeShape[1] == 4 || runtimeShape[1] <= 0;
		const bool mayBeNhwc = runtimeShape[3] == 3 || runtimeShape[3] == 4 || runtimeShape[3] <= 0;

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

	bool ResolveRuntimeImageSize(const std::vector<int64_t>& runtimeShape, int& width, int& height) {
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

	bool TryResolveModelImageSize(const TakeC::OnnxModel* model, int& width, int& height) {
		if (!model || model->GetInputShapes().empty()) {
			return false;
		}

		std::vector<int64_t> runtimeShape;
		if (!ResolveRuntimeImageInputShapeWithFallback(model->GetInputShapes()[0], 192, 192, runtimeShape)) {
			return false;
		}

		return ResolveRuntimeImageSize(runtimeShape, width, height);
	}

	void DrawFaceDetectionResult(const FaceDetectionResult& result, size_t index) {
		ImGui::Text("Face %d score: %.3f", static_cast<int>(index), result.score);
		ImGui::Text("bbox: min(%.1f, %.1f) max(%.1f, %.1f)",
			result.bboxMin.x,
			result.bboxMin.y,
			result.bboxMax.x,
			result.bboxMax.y);
		ImGui::Text("leftEye : %.1f, %.1f", result.landmark.leftEye.x, result.landmark.leftEye.y);
		ImGui::Text("rightEye: %.1f, %.1f", result.landmark.rightEye.x, result.landmark.rightEye.y);
		ImGui::Text("nose    : %.1f, %.1f", result.landmark.nose.x, result.landmark.nose.y);
		ImGui::Text("leftMouth : %.1f, %.1f", result.landmark.leftMouth.x, result.landmark.leftMouth.y);
		ImGui::Text("rightMouth: %.1f, %.1f", result.landmark.rightMouth.x, result.landmark.rightMouth.y);
		ImGui::Separator();
	}

	std::vector<Vector2> DecodeLandmark106Points(
		const std::vector<std::vector<float>>& outputDataList,
		int alignedWidth,
		int alignedHeight,
		bool outputMinusOneToOne) {

		std::vector<Vector2> points;
		if (outputDataList.empty() || outputDataList[0].size() < 212 || alignedWidth <= 0 || alignedHeight <= 0) {
			return points;
		}

		const std::vector<float>& output = outputDataList[0];
		points.reserve(106);
		for (size_t i = 0; i < 106; ++i) {
			float x = output[i * 2 + 0];
			float y = output[i * 2 + 1];

			if (outputMinusOneToOne) {
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

	//================================================================
	//		顔検出結果のオーバーレイ描画
	//================================================================
	void DrawFaceDetectionOverlay(
		const TakeC::CameraCapture& cameraCapture,
		const std::vector<FaceDetectionResult>& results,
		int inputWidth,
		int inputHeight) {

		if (!cameraCapture.HasLastImGuiImageRect() || inputWidth <= 0 || inputHeight <= 0) {
			return;
		}

		const TakeC::CameraCapture::ImGuiImageRect& rect = cameraCapture.GetLastImGuiImageRect();
		const float imageWidth = rect.maxX - rect.minX;
		const float imageHeight = rect.maxY - rect.minY;
		if (imageWidth <= 0.0f || imageHeight <= 0.0f) {
			return;
		}

		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		const ImU32 boxColor = IM_COL32(0, 255, 80, 255);
		const ImU32 landmarkColor = IM_COL32(255, 80, 40, 255);
		const ImU32 textColor = IM_COL32(255, 255, 255, 255);
		const float scaleX = imageWidth / static_cast<float>(inputWidth);
		const float scaleY = imageHeight / static_cast<float>(inputHeight);

		const auto toScreen = [&](const Vector2& point) {
			return ImVec2(rect.minX + point.x * scaleX, rect.minY + point.y * scaleY);
			};

		// 顔検出結果の描画
		for (size_t i = 0; i < results.size(); ++i) {
			const FaceDetectionResult& result = results[i];
			const ImVec2 min = toScreen(result.bboxMin);
			const ImVec2 max = toScreen(result.bboxMax);
			drawList->AddRect(min, max, boxColor, 0.0f, 0, 2.0f);
			drawList->AddText(ImVec2(min.x, min.y - 16.0f), textColor, std::to_string(i).c_str());

			const Vector2 landmarks[5] = {
				result.landmark.leftEye,
				result.landmark.rightEye,
				result.landmark.nose,
				result.landmark.leftMouth,
				result.landmark.rightMouth,
			};

			for (const Vector2& landmark : landmarks) {
				const ImVec2 center = toScreen(landmark);
				drawList->AddCircle(center, 4.0f, landmarkColor, 12, 2.0f);
				drawList->AddLine(ImVec2(center.x - 5.0f, center.y), ImVec2(center.x + 5.0f, center.y), landmarkColor, 1.5f);
				drawList->AddLine(ImVec2(center.x, center.y - 5.0f), ImVec2(center.x, center.y + 5.0f), landmarkColor, 1.5f);
			}
		}
	}

	void DrawLandmark106Canvas(const std::vector<Vector2>& points, int width, int height) {
		if (points.empty() || width <= 0 || height <= 0 || !ImGui::TreeNode("106 Landmark Preview")) {
			return;
		}

		const float canvasSize = std::min(ImGui::GetContentRegionAvail().x, 360.0f);
		const ImVec2 origin = ImGui::GetCursorScreenPos();
		const ImVec2 canvasEnd = ImVec2(origin.x + canvasSize, origin.y + canvasSize);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(origin, canvasEnd, IM_COL32(20, 20, 20, 255));
		drawList->AddRect(origin, canvasEnd, IM_COL32(120, 120, 120, 255));

		const float scaleX = canvasSize / static_cast<float>(width);
		const float scaleY = canvasSize / static_cast<float>(height);
		for (size_t i = 0; i < points.size(); ++i) {
			const ImVec2 p = ImVec2(origin.x + points[i].x * scaleX, origin.y + points[i].y * scaleY);
			drawList->AddCircleFilled(p, 2.0f, IM_COL32(255, 180, 40, 255), 8);
			if (i == 0 || i == 105) {
				drawList->AddText(ImVec2(p.x + 3.0f, p.y + 3.0f), IM_COL32(255, 255, 255, 255), std::to_string(i).c_str());
			}
		}

		ImGui::Dummy(ImVec2(canvasSize, canvasSize));
		ImGui::TreePop();
	}
}

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
		TryResolveModelImageSize(debugLandmarkModel_, landmarkInputWidth, landmarkInputHeight)) {
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
			if (ResolveRuntimeImageInputShape(modelInputShapes[0], runtimeInputShape) &&
				debugOnnxModel_->ResizeInputBuffer(0, runtimeInputShape)) {

				std::vector<float>* input = debugOnnxModel_->GetInputData(0);
				if (input && TakeC::BuildFaceInputFromCamera(*cameraCapture, *input, runtimeInputShape)) {
					debugOnnxRunSuccess_ = debugOnnxModel_->Run();
					if (debugOnnxRunSuccess_) {
						int inputWidth = 0;
						int inputHeight = 0;
						if (ResolveRuntimeImageSize(runtimeInputShape, inputWidth, inputHeight)) {
							ScrfdDecoder::Config config;
							config.inputWidth = inputWidth;
							config.inputHeight = inputHeight;
							config.scoreThreshold = debugScrfdScoreThreshold_;
							config.nmsThreshold = debugScrfdNmsThreshold_;
							config.multiplyByStride = debugScrfdMultiplyByStride_;
							config.anchorCenterOffset = debugScrfdAnchorCenterOffset_;
							config.decodeOffsetX = debugScrfdDecodeOffsetX_;
							config.decodeOffsetY = debugScrfdDecodeOffsetY_;
							debugFaceResults_ = debugScrfdDecoder_.Decode(
								debugOnnxModel_->GetOutputNames(),
								debugOnnxModel_->GetOutputDataList(),
								debugOnnxModel_->GetRuntimeOutputShapes(),
								config);
							debugScrfdInputWidth_ = inputWidth;
							debugScrfdInputHeight_ = inputHeight;

							debugFaceAlignSuccess_ = false;
							debugAlignedFaceImage_ = {};
							if (!debugFaceResults_.empty()) {
								FaceAligner::Config alignConfig;
								alignConfig.outputWidth = debugAlignedFaceSize_;
								alignConfig.outputHeight = debugAlignedFaceSize_;
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
							if (debugFaceAlignSuccess_ && debugLandmarkModel_) {
								const auto& landmarkModelInputShapes = debugLandmarkModel_->GetInputShapes();
								if (!landmarkModelInputShapes.empty()) {
									std::vector<int64_t> landmarkRuntimeInputShape;
									if (ResolveRuntimeImageInputShapeWithFallback(
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
												debugLandmark106Points_ = DecodeLandmark106Points(
													debugLandmarkModel_->GetOutputDataList(),
													debugAlignedFaceImage_.width,
													debugAlignedFaceImage_.height,
													debugLandmarkOutputMinusOneToOne_);
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
		DrawFaceDetectionOverlay(*cameraCapture, debugFaceResults_, debugScrfdInputWidth_, debugScrfdInputHeight_);
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
			TryResolveModelImageSize(debugLandmarkModel_, landmarkInputWidth, landmarkInputHeight)) {
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

		DrawOnnxTensorInfo("Model Inputs", debugOnnxModel_->GetInputNames(), debugOnnxModel_->GetInputShapes());
		DrawOnnxTensorInfo("Model Outputs", debugOnnxModel_->GetOutputNames(), debugOnnxModel_->GetOutputShapes());
		DrawOnnxTensorInfo("Runtime Inputs", debugOnnxModel_->GetInputNames(), debugOnnxModel_->GetRuntimeInputShapes());
		DrawOnnxTensorInfo("Runtime Outputs", debugOnnxModel_->GetOutputNames(), debugOnnxModel_->GetRuntimeOutputShapes());

		if (debugLandmarkModel_) {
			ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Landmark model loaded.");
			ImGui::Text("Landmark Run: %s", debugLandmarkRunSuccess_ ? "success" : "not run / failed");
			DrawOnnxTensorInfo("Landmark Model Inputs", debugLandmarkModel_->GetInputNames(), debugLandmarkModel_->GetInputShapes());
			DrawOnnxTensorInfo("Landmark Model Outputs", debugLandmarkModel_->GetOutputNames(), debugLandmarkModel_->GetOutputShapes());
			DrawOnnxTensorInfo("Landmark Runtime Inputs", debugLandmarkModel_->GetInputNames(), debugLandmarkModel_->GetRuntimeInputShapes());
			DrawOnnxTensorInfo("Landmark Runtime Outputs", debugLandmarkModel_->GetOutputNames(), debugLandmarkModel_->GetRuntimeOutputShapes());
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
			DrawLandmark106Canvas(debugLandmark106Points_, debugAlignedFaceImage_.width, debugAlignedFaceImage_.height);
		}
		const size_t displayCount = std::min<size_t>(debugFaceResults_.size(), 5);
		for (size_t i = 0; i < displayCount; ++i) {
			DrawFaceDetectionResult(debugFaceResults_[i], i);
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
