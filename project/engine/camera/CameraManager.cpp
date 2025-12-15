#include "CameraManager.h"
#include "engine/Base/ImGuiManager.h"

using namespace TakeC;

//============================================================================
// インスタンスの取得
//============================================================================
CameraManager& CameraManager::GetInstance() {
	static CameraManager instance;
	return instance;
}

//============================================================================
// 初期化
//============================================================================
void CameraManager::Initialize(TakeC::DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

//============================================================================
// 更新処理
//============================================================================
void CameraManager::Update() {
	if (activeCamera_) {
		// アクティブカメラの更新
		activeCamera_->Update();
	}
}

//============================================================================
// ImGuiの更新
//============================================================================
void CameraManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("CameraManager");
	ImGui::Text("Camera Count : %d", cameras_.size());
	ImGui::Separator();
	for (const auto& [name, camera] : cameras_) {
		// ボタンを表示し、押されたらアクティブカメラを設定
		if (ImGui::Button(name.c_str())) {
			SetActiveCamera(name);
		}
	}
	activeCamera_->UpdateImGui();
	ImGui::End();
#endif // _DEBUG
}

//============================================================================
// 終了・開放処理
//============================================================================
void CameraManager::Finalize() {
	cameras_.clear();
}

//============================================================================
// カメラの追加
//============================================================================
void CameraManager::AddCamera(std::string name, const Camera& camera) {
	//生成済みのカメラの検索
	if (cameras_.contains(name)) {
		return;
	}

	// 新しいカメラを追加
	cameras_.insert(std::make_pair(name, std::make_unique<Camera>(camera)));

	// もしこれが最初のカメラなら、それをアクティブに設定
	if (cameras_.size() == 1) {
		activeCamera_ = cameras_.find(name)->second.get();
	}
}

//============================================================================
// カメラのリセット
//============================================================================
void CameraManager::ResetCameras() {
	cameras_.clear();
	activeCamera_ = nullptr;
}

//============================================================================
// 登録されたカメラの取得
//============================================================================
Camera* TakeC::CameraManager::FindCameraByName(std::string name) {
	auto it = cameras_.find(name);
	if (it != cameras_.end()) {
		return it->second.get();
	}
	return nullptr;
}

//============================================================================
// アクティブカメラの設定
//============================================================================
void CameraManager::SetActiveCamera(std::string name) {

	auto it = cameras_.find(name);
	// 指定された名前のカメラが見つかった場合は、それをアクティブに設定
	if (it != cameras_.end()) {
		activeCamera_ = it->second.get();
	} else {
		// 見つからなかった場合は、最初のカメラをアクティブに設定
		it = cameras_.begin();
	}
}

//============================================================================
// アクティブカメラの取得
//============================================================================
Camera* CameraManager::GetActiveCamera() {
	return activeCamera_;
}

//============================================================================
// カメラの数を取得
//============================================================================
int CameraManager::GetCameraCount() const {
	return static_cast<int>(cameras_.size());
}