#include "BulletManager.h"

//========================================================================================================
//　初期化
//========================================================================================================

void BulletManager::Initialize(Object3dCommon* object3dCommon,size_t size) {
	object3dCommon_ = object3dCommon;

	bulletEffectConfig_.trailEffectFilePath.push_back("BulletMoveEffect.json");
	bulletEffectConfig_.explosionEffectFilePath.push_back("DamageSpark2.json");

	missileEffectConfig_.trailEffectFilePath.push_back("MissileSmoke.json");
	missileEffectConfig_.explosionEffectFilePath.push_back("MissileExplosion.json");

	bazookaBulletEffectConfig_.trailEffectFilePath.push_back("BazookaBulletTrail.json");
	bazookaBulletEffectConfig_.explosionEffectFilePath.push_back("BazookaBulletExplosion1.json");
	bazookaBulletEffectConfig_.explosionEffectFilePath.push_back("BazookaBulletExplosion2.json");

	bulletFilePath_ = "Bullet.gltf";
	missileFilePath_ = "Missile.gltf";
	bazookaBulletFilePath_ = "BazookaBullet.gltf";
	bulletPool_ = std::make_unique<BulletPool>();
	bulletPool_->Initialize(object3dCommon_,size,bulletFilePath_, bulletEffectConfig_);
	missilePool_ = std::make_unique<MissilePool>();
	missilePool_->Initialize(object3dCommon_,size * 2,missileFilePath_, missileEffectConfig_);
	bazookaBulletPool_ = std::make_unique<BulletPool>();
	bazookaBulletPool_->Initialize(object3dCommon_, size,bazookaBulletFilePath_, bazookaBulletEffectConfig_);

}

//========================================================================================================
// 終了処理
//========================================================================================================

void BulletManager::Finalize() {

	missilePool_->Finalize();
	bulletPool_->Finalize();
	bazookaBulletPool_->Finalize();
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
	// バズーカ弾の更新
	bazookaBulletPool_->UpdateAll();
}



//========================================================================================================
// 各弾の描画処理
//========================================================================================================

void BulletManager::Draw() {

	// 弾の描画
	bulletPool_->DrawAll();
	// ミサイルの描画
	missilePool_->DrawAll();
	// バズーカ弾の描画
	bazookaBulletPool_->DrawAll();
}

void BulletManager::DrawCollider() {

	bulletPool_->DrawAllCollider();
	missilePool_->DrawAllCollider();
	bazookaBulletPool_->DrawAllCollider();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,const Vector3& targetVel,float speed,float power,CharacterType type) {

	Bullet* bullet = bulletPool_->GetObject();
	bullet->Create(weaponPos, targetPos,targetVel,speed,power,type);
}

void BulletManager::ShootBullet(const Vector3& weaponPos, const Vector3& direction,float speed, float power, CharacterType type) {

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

void BulletManager::ShootBazookaBullet(
	const Vector3& weaponPos,
	const Vector3& targetPos,
	const Vector3& targetVel,
	float speed, float power,
	CharacterType type) {

	Bullet* bazookaBullet = bazookaBulletPool_->GetObject();
	bazookaBullet->Create(weaponPos, targetPos, targetVel, speed, power, type);
}

void BulletManager::ShootBazookaBullet(
	const Vector3& weaponPos,
	const Vector3& direction,
	float speed, float power,
	CharacterType type) {

	Bullet* bazookaBullet = bazookaBulletPool_->GetObject();
	bazookaBullet->Create(weaponPos, direction, speed, power, type);
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

//========================================================================================================
// 全バズーカ弾の取得
//========================================================================================================
std::vector<Bullet*> BulletManager::GetAllBazookaBullets() {
	
	std::vector<Bullet*> bazookaBullets;
	for (const auto& bullet : bazookaBulletPool_->GetPool()) {
		if (bullet->IsActive()) {
			bazookaBullets.push_back(bullet);
		}
	}
	return bazookaBullets;
}
