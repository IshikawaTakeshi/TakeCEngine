#include "PlayerReticle.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/WinApp.h"
#include "engine/camera/CameraManager.h"
#include "engine/math/MatrixMath.h"
#include "engine/math/Vector3Math.h"

void PlayerReticle::Initialize() {
		// レティクルのスプライトを初期化
	reticleSprite_ = std::make_unique<Sprite>();
	reticleSprite_->Initialize(SpriteCommon::GetInstance(), "reticle_focusTarget.png");
	reticleSprite_->SetAnchorPoint({ 0.5f, 0.5f }); // アンカーポイントを中心に設定
	reticleSprite_->AdjustTextureSize(); // テクスチャサイズをイメージに合わせる
	reticleSprite_->SetSize({ size_, size_ }); // レティクルのサイズを設定
	screenPosition_ = { WinApp::kScreenWidth / 2.0f, WinApp::kScreenHeight / 2.0f };
}

void PlayerReticle::Update(const Vector3& targetPosition) {

	targetPosition_ = targetPosition;

	//ロックオン時
	if (isFocus_) {
		Matrix4x4 viewProjectionMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
		Vector3 screenPos = MatrixMath::Transform(targetPosition_, viewProjectionMatrix);
		screenPosition_ = {
			(screenPos.x + 1.0f) * WinApp::kScreenWidth / 2.0f,
			(1.0f - screenPos.y) * WinApp::kScreenHeight / 2.0f
		};
		
	} else {
		targetPosition_ = { WinApp::kScreenWidth / 2.0f, WinApp::kScreenHeight / 2.0f };
	}

	// スプライトの位置とサイズを更新
	reticleSprite_->SetPosition(screenPosition_);
	reticleSprite_->SetSize({ size_, size_ });
	reticleSprite_->Update();
}

void PlayerReticle::Draw() {
	reticleSprite_->Draw();
}	

void PlayerReticle::UpdateImGui() {
	reticleSprite_->UpdateImGui("Reticle Sprite");
}