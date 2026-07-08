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
	TakeC::TakeCFrameWork::GetCameraCapture()->UpdateImGui();

	ImGui::Begin("ONNX Model Debug");
	ImGui::InputText("Model Path", debugOnnxModelPath_.data(), debugOnnxModelPath_.size());
	if (ImGui::Button("Load ONNX Model")) {
		debugOnnxModel_ = TakeC::TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
			"TitleSceneDebugModel",
			StringUtility::ConvertString(debugOnnxModelPath_.data()));
		debugOnnxLoadFailed_ = debugOnnxModel_ == nullptr;
	}

	if (debugOnnxModel_) {
		ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Loaded");
		ImGui::Checkbox("Enable Inference", &debugOnnxInferenceEnabled_);
		ImGui::SliderFloat("Inference Interval", &debugOnnxInferenceInterval_, 0.0f, 1.0f, "%.3f sec");

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
