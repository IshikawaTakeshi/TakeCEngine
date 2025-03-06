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
		if (Input::GetInstance()->PushKey(DIK_W)) {
			object3d_->SetPosition(object3d_->GetTranslate() + Vector3(0.0f, 0.0f, 0.1f));
		}

		if (Input::GetInstance()->PushKey(DIK_S)) {
			object3d_->SetPosition(object3d_->GetTranslate() + Vector3(0.0f, 0.0f, -0.1f));
		}

		if (Input::GetInstance()->PushKey(DIK_A)) {
			object3d_->SetPosition(object3d_->GetTranslate() + Vector3(-0.1f, 0.0f, 0.0f));
		}

		if (Input::GetInstance()->PushKey(DIK_D)) {
			object3d_->SetPosition(object3d_->GetTranslate() + Vector3(0.1f, 0.0f, 0.0f));
		}
	}
	//キー入力で移動
	object3d_->Update();

#ifdef _DEBUG
	collider_->Update();
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
	}
}

void SampleCharacter::SkinningDisPatch() {

	object3d_->DisPatch();
}
