#include "TitleScene.h"
#include <algorithm>
#include <string>
#include <vector>

#include "application/Scene/SceneManager.h"

#include "engine/base/TakeCFrameWork.h"
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
}

//====================================================================
//			初期化
//====================================================================
void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_unique<Camera>();
	camera0_->Initialize(TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),"Title_ViewCamera.json");
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
	TakeCFrameWork::GetPostEffectManager()->SetEffectActive("ShadowMapEffect", false);
	TakeCFrameWork::GetPostEffectManager()->SetEffectActive("DepthBasedOutline", false);

	debugOnnxModel_ = TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
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
	TakeCFrameWork::GetSpriteManager()->Clear();
}

//====================================================================
//			更新処理
//====================================================================
void TitleScene::Update() {

	//カメラの更新
	TakeC::CameraManager::GetInstance().Update();

	//SkyBoxの更新
	skyBox_->Update();

	//シーン遷移
	if (TakeC::Input::GetInstance().TriggerButton(0,GamepadButtonType::A)) {
		//シーン切り替え依頼
		//EnemySelectSceneへ
		SceneManager::GetInstance().ChangeScene("GAMEPLAY",1.0f);
	}
}

//====================================================================
//			ImGui更新処理
//====================================================================
void TitleScene::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	//ImGuiの更新
	TakeC::CameraManager::GetInstance().UpdateImGui();
	TakeCFrameWork::GetSpriteManager()->UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();

	ImGui::Begin("ONNX Model Debug");
	ImGui::InputText("Model Path", debugOnnxModelPath_.data(), debugOnnxModelPath_.size());
	if (ImGui::Button("Load ONNX Model")) {
		debugOnnxModel_ = TakeCFrameWork::GetOnnxRuntimeSystem()->LoadModel(
			"TitleSceneDebugModel",
			StringUtility::ConvertString(debugOnnxModelPath_.data()));
		debugOnnxLoadFailed_ = debugOnnxModel_ == nullptr;
	}

	if (debugOnnxModel_) {
		ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Loaded");
		DrawOnnxTensorInfo("Inputs", debugOnnxModel_->GetInputNames(), debugOnnxModel_->GetInputShapes());
		DrawOnnxTensorInfo("Outputs", debugOnnxModel_->GetOutputNames(), debugOnnxModel_->GetOutputShapes());
	} else if (debugOnnxLoadFailed_) {
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "Load failed. Check model path and Output window log.");
	} else {
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
