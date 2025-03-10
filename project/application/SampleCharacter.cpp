#include "SampleCharacter.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Model.h"
#include "Input.h"

void SampleCharacter::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//キャラクタータイプ設定
	characterType_ = CharacterType::PLAYER;
	
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
}

void SampleCharacter::Update() {


	if(characterType_ == CharacterType::PLAYER) {
		XINPUT_STATE joystickState;
		if (Input::GetInstance()->GetJoystickState(0, joystickState)) { // 0番のジョイスティックを使用
			// 左スティックの入力値（-32768 ～ 32767 の範囲）
			float x = static_cast<float>(joystickState.Gamepad.sThumbLX);
			float y = static_cast<float>(joystickState.Gamepad.sThumbLY);

			// スティックのデッドゾーン処理（XINPUT ゲームパッドの仕様に準拠）
			constexpr float DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			float magnitude = sqrtf(x * x + y * y);

			if (magnitude > DEADZONE) {
				// デッドゾーンを補正して 0.0f ~ 1.0f の範囲に正規化
				float normalizedX = x / 32768.0f;
				float normalizedY = y / 32768.0f;

				// 移動速度を適用
				constexpr float SPEED = 1.0f;
				object3d_->SetTranslate({
					object3d_->GetTranslate().x + normalizedX * SPEED,
					object3d_->GetTranslate().y,
					object3d_->GetTranslate().z
					});

				object3d_->SetTranslate({
					object3d_->GetTranslate().x,
					object3d_->GetTranslate().y,
					object3d_->GetTranslate().z + normalizedY * SPEED
					});
				
			} 
		}
	}

	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });

	//キー入力で移動
	object3d_->Update();

#ifdef _DEBUG
	collider_->Update(object3d_->GetTransform());
#endif // _DEBUG

}

void SampleCharacter::Draw() {

	object3d_->Draw();

}

void SampleCharacter::DrawCollider() {

	collider_->DrawCollider();
}

//=============================================================================
// 衝突時の処理
//=============================================================================

void SampleCharacter::OnCollisionAction(GameCharacter* other) {

	
	if (other->GetCharacterType() == CharacterType::ENEMY) {
		//衝突したら赤色に変更
		object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 1.0f,0.0f,0.0f,1.0f });
		collider_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 1.0f,0.0f,0.0f,1.0f });
	}
}

void SampleCharacter::SkinningDisPatch() {

	object3d_->DisPatch();
}
