#include "Rifle.h"

//最大弾数の設定


Rifle::~Rifle() {}

void Rifle::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//武器の初期化
	weaponType_ = 0;
	attackPower_ = 10;
	attackInterval_ = 0.5f;
	bulletCount_ = 30;
	maxBulletCount_ = 30;

}

void Rifle::Update() {

	object3d_->Update();
}

void Rifle::Draw() {

	object3d_->Draw();
}

void Rifle::Attack() {


}
