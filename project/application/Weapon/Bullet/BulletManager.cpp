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
	bulletFilePath_ = "Bullet.gltf";
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
// 各弾の描画処理
//========================================================================================================

void BulletManager::DrawBullet() {
	// 弾の描画
	bulletPool_->DrawAllBullet();
}

void BulletManager::DrawMissile() {
	// ミサイルの描画
	missilePool_->DrawAllMissiles();
}

void BulletManager::DrawCollider() {

	bulletPool_->DrawAllCollider();
	missilePool_->DrawAllCollider();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,const Vector3& targetVel,const float& speed,float power,CharacterType type) {

	Bullet* bullet = bulletPool_->GetBullet();
	//bullet->Initialize(object3dCommon_, bulletFilePath_);
	bullet->Create(weaponPos, targetPos,targetVel,speed,power,type);
}

//========================================================================================================
// ミサイルの発射処理
//========================================================================================================
void BulletManager::ShootMissile(BaseWeapon* ownerWeapon,VerticalMissileInfo vmInfo, float speed,float power, CharacterType type) {

	VerticalMissile* missile = missilePool_->GetMissile();
	if (missile == nullptr) {
		return; // ミサイルが取得できなかった場合は何もしない
	}
	missile->Initialize(object3dCommon_, missileFilePath_);
	missile->Create(ownerWeapon,vmInfo, speed,power, type);
}

//========================================================================================================
// 全弾の取得
//========================================================================================================
std::vector<Bullet*> BulletManager::GetAllBullets() {
	
	std::vector<Bullet*> bullets;
	for (const auto& bullet : bulletPool_->GetPool()) {
		if (bullet->GetIsActive()) {
			bullets.push_back(bullet);
		}
	}
	return bullets;
}

//========================================================================================================
// 全ミサイルの取得
//========================================================================================================
std::vector<VerticalMissile*> BulletManager::GetAllMissiles() {
	
	std::vector<VerticalMissile*> missiles;
	for (const auto& missile : missilePool_->GetPool()) {
		if (missile->GetIsActive()) {
			missiles.push_back(missile);
		}
	}
	return missiles;
}
