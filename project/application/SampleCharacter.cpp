#include "SampleCharacter.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Model.h"

void SampleCharacter::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
}

void SampleCharacter::Update() {

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

	//衝突したら赤色に変更
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 1.0f,0.0f,0.0f,1.0f });

	if (other->GetCharacterType() == CharacterType::ENEMY) {
		//敵と衝突したら消滅
		object3d_->SetPosition({ 0.0f,0.0f,0.0f });
	}
}
