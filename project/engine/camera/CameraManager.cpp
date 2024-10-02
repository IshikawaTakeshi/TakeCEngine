#include "CameraManager.h"
#include "Camera.h"

#pragma region imgui
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif 
#pragma endregion

CameraManager* CameraManager::instance_ = nullptr;

CameraManager* CameraManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CameraManager();
	}
	return instance_;
}

void CameraManager::Update() {
	if (activeCamera_) {
		activeCamera_->Update();
	}
}

void CameraManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("CameraManager");
	ImGui::Text("Camera Count : %d", cameras_.size());
	ImGui::Separator();
	for (int i = 0; i < static_cast<int>(cameras_.size()); i++) {
		if (ImGui::Button(("Camera" + std::to_string(i)).c_str())) {
			SetActiveCamera(i);
		}
	}
	activeCamera_->UpdateImGui();
	ImGui::End();
#endif // _DEBUG
}

void CameraManager::Finalize() {
	delete instance_;
	instance_ = nullptr;
}

void CameraManager::AddCamera(const Camera& camera) {
	// カメラを追加
	cameras_.push_back(std::make_unique<Camera>(camera));

	// もしこれが最初のカメラなら、それをアクティブに設定
	if (cameras_.size() == 1) {
		activeCamera_ = cameras_.back().get();
	}
}

void CameraManager::SetActiveCamera(int index) {
	if (index >= 0 && index < static_cast<int>(cameras_.size())) {
		activeCamera_ = cameras_[index].get();
	}
}

Camera* CameraManager::GetActiveCamera() {
	return activeCamera_;
}

int CameraManager::GetCameraCount() const {
	return static_cast<int>(cameras_.size());
}