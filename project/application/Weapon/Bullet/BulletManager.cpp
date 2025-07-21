#include "BulletManager.h"

//========================================================================================================
//　初期化
//========================================================================================================

void BulletManager::Initialize(Object3dCommon* object3dCommon,size_t size) {
	bulletPool_ = std::make_unique<BulletPool>();
	bulletPool_->Initialize(size);
	missilePool_ = std::make_unique<MissilePool>();
	missilePool_->Initialize(size); 
	object3dCommon_ = object3dCommon;
	bulletFilePath_ = "cube.obj";
	missileFilePath_ = "ICOBall.gltf";
}

//========================================================================================================
// 終了処理
//========================================================================================================

void BulletManager::Finalize() {

	bulletPool_->Finalize();
	missilePool_->Finalize();
	object3dCommon_ = nullptr;
}

//========================================================================================================
// 全弾の更新処理
//========================================================================================================

void BulletManager::Update() {
	// 弾の更新
	bulletPool_->UpdateAllBullet();
	// ミサイルの更新
	missilePool_->UpdateAllMissiles();
}

//========================================================================================================
// 全弾の描画処理
//========================================================================================================

void BulletManager::Draw() {
	bulletPool_->DrawAllBullet();
	missilePool_->DrawAllMissiles();
}

void BulletManager::DrawCollider() {

	bulletPool_->DrawAllCollider();
	missilePool_->DrawAllCollider();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,const float& speed,CharacterType type) {

	Bullet* bullet = bulletPool_->GetBullet();
	bullet->Initialize(object3dCommon_, bulletFilePath_);
	bullet->Create(weaponPos, targetPos,speed,type);
	//bullet->EmitterInitialize(10, 0.1f); // 10個のパーティクルを0.1秒間隔で発生させる
}

void BulletManager::ShootMissile(BaseWeapon* ownerWeapon, const float& speed, CharacterType type) {

	VerticalMissile* missile = missilePool_->GetMissile();
	if (missile == nullptr) {
		return; // ミサイルが取得できなかった場合は何もしない
	}
	missile->Initialize(object3dCommon_, missileFilePath_);
	missile->Create(ownerWeapon, speed, type);
}

std::vector<Bullet*> BulletManager::GetAllBullets() {
	
	std::vector<Bullet*> bullets;
	for (const auto& bullet : bulletPool_->GetPool()) {
		if (bullet->GetIsActive()) {
			bullets.push_back(bullet);
		}
	}
	return bullets;
}

std::vector<VerticalMissile*> BulletManager::GetAllMissiles() {
	
	std::vector<VerticalMissile*> missiles;
	for (const auto& missile : missilePool_->GetPool()) {
		if (missile->GetIsActive()) {
			missiles.push_back(missile);
		}
	}
	return missiles;
}
