#include "PlayerReticle.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/WinApp.h"
#include "engine/camera/CameraManager.h"
#include "engine/math/MatrixMath.h"
#include "engine/math/Vector3Math.h"

//===================================================================================
//　初期化
//===================================================================================
void PlayerReticle::Initialize() {
		// レティクルのスプライトを初期化
	reticleSprite_ = std::make_unique<Sprite>();
	reticleSprite_->Initialize(SpriteCommon::GetInstance(), "reticle_focusTarget.png");
	reticleSprite_->SetAnchorPoint({ 0.5f, 0.5f }); // アンカーポイントを中心に設定
	reticleSprite_->AdjustTextureSize(); // テクスチャサイズをイメージに合わせる
	reticleSprite_->SetSize({ size_, size_ }); // レティクルのサイズを設定
	screenPosition_ = { WinApp::kScreenWidth / 2.0f, WinApp::kScreenHeight / 2.0f };
	targetPosition_ = { 0.0f, 0.0f, 0.0f };
}

//===================================================================================
//　更新
//===================================================================================
void PlayerReticle::Update(const Vector3& targetPosition) {

	//ターゲット位置の取得
	targetPosition_ = targetPosition;

	//ロックオン時
	if (isFocus_) {

		//ターゲットのワールド座標をスクリーン座標に変換
		Matrix4x4 viewProjectionMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
		Vector3 screenPos = MatrixMath::Transform(targetPosition_, viewProjectionMatrix);

		//スクリーン座標に変換（-1～1の範囲）
		screenPosition_ = {
			(screenPos.x + 1.0f) * WinApp::kScreenWidth / 2.0f,
			(1.0f - screenPos.y) * WinApp::kScreenHeight / 2.0f
		};
		
	} else {
		//非ロックオン時は画面中央に表示
		targetPosition_ = { WinApp::kScreenWidth / 2.0f, WinApp::kScreenHeight / 2.0f };
	}

	// スプライトの位置とサイズを更新
	reticleSprite_->SetPosition(screenPosition_);
	reticleSprite_->SetSize({ size_, size_ });
	reticleSprite_->Update();
}

//===================================================================================
//　描画
//===================================================================================
void PlayerReticle::Draw() {
	reticleSprite_->Draw();
}	


//===================================================================================
//　ImGuiの更新
//===================================================================================
void PlayerReticle::UpdateImGui() {
#ifdef _DEBUG
	reticleSprite_->UpdateImGui("Reticle Sprite");
#endif
}