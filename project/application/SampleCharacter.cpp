#include "SampleCharacter.h"
#include "Collision/BoxCollider.h"

void SampleCharacter::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3d_.get(), filePath);
}

void SampleCharacter::Update() {

	object3d_->Update();

	collider_
}
