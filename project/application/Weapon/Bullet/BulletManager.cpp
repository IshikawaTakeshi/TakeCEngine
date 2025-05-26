#include "BulletManager.h"

//========================================================================================================
//　初期化
//========================================================================================================

void BulletManager::Initialize(Object3dCommon* object3dCommon,size_t size) {

	bulletPool_.Initialize(size);
	object3dCommon_ = object3dCommon;
	bulletFilePath_ = "cube.obj";
}

//========================================================================================================
// 終了処理
//========================================================================================================

void BulletManager::Finalize() {

	bulletPool_.Finalize();
	object3dCommon_ = nullptr;
}

//========================================================================================================
// 全弾の更新処理
//========================================================================================================

void BulletManager::UpdateBullet() {
	bulletPool_.UpdateAllBullet();
}

//========================================================================================================
// 全弾の描画処理
//========================================================================================================

void BulletManager::DrawBullet() {
	bulletPool_.DrawAllBullet();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,CharacterType type) {

	Bullet* bullet = bulletPool_.GetBullet();
	if (bullet) {
		bullet->Initialize(object3dCommon_, bulletFilePath_);
		bullet->BulletInitialize(weaponPos, targetPos,type);
	}
}
