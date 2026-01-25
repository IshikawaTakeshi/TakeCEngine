#include "BulletManager.h"

//========================================================================================================
//　初期化
//========================================================================================================

void BulletManager::Initialize(Object3dCommon* object3dCommon,size_t size) {
	object3dCommon_ = object3dCommon;
	bulletPool_ = std::make_unique<BulletPool>();
	bulletPool_->Initialize(object3dCommon_,size);
	missilePool_ = std::make_unique<MissilePool>();
	missilePool_->Initialize(object3dCommon_,size * 2); 
	bulletFilePath_ = "Bullet.gltf";
	missileFilePath_ = "ICOBall.gltf";
}

//========================================================================================================
// 終了処理
//========================================================================================================

void BulletManager::Finalize() {

	missilePool_->Finalize();
	bulletPool_->Finalize();
	object3dCommon_ = nullptr;
}

//========================================================================================================
// 全弾の更新処理
//========================================================================================================

void BulletManager::Update() {
	// 弾の更新
	bulletPool_->UpdateAll();
	// ミサイルの更新
	missilePool_->UpdateAll();
}



//========================================================================================================
// 各弾の描画処理
//========================================================================================================

void BulletManager::DrawBullet() {
	// 弾の描画
	bulletPool_->DrawAll();
}

void BulletManager::DrawMissile() {
	// ミサイルの描画
	missilePool_->DrawAll();
}

void BulletManager::DrawCollider() {

	bulletPool_->DrawAllCollider();
	missilePool_->DrawAllCollider();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,const Vector3& targetVel,const float& speed,float power,CharacterType type) {

	Bullet* bullet = bulletPool_->GetObject();
	bullet->Create(weaponPos, targetPos,targetVel,speed,power,type);
}

void BulletManager::ShootBullet(const Vector3& weaponPos, const Vector3& direction, const float& speed, float power, CharacterType type) {

	Bullet* bullet = bulletPool_->GetObject();
	bullet->Create(weaponPos, direction, speed, power, type);
}

//========================================================================================================
// ミサイルの発射処理
//========================================================================================================
void BulletManager::ShootMissile(BaseWeapon* ownerWeapon,VerticalMissileInfo vmInfo, float speed,float power, CharacterType type) {

	VerticalMissile* missile = missilePool_->GetObject();
	if (missile == nullptr) {
		return; // ミサイルが取得できなかった場合は何もしない
	}
	missile->Create(ownerWeapon,vmInfo, speed,power, type);
}

//========================================================================================================
// 全弾の取得
//========================================================================================================
std::vector<Bullet*> BulletManager::GetAllBullets() {
	
	std::vector<Bullet*> bullets;
	for (const auto& bullet : bulletPool_->GetPool()) {
		if (bullet->IsActive()) {
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
		if (missile->IsActive()) {
			missiles.push_back(missile);
		}
	}
	return missiles;
}
