#include "BulletManager.h"

//========================================================================================================
//　初期化
//========================================================================================================

void BulletManager::Initialize(Object3dCommon* object3dCommon,size_t size) {
	object3dCommon_ = object3dCommon;

	playerBulletEffectConfig_.trailEffectFilePath.push_back("BulletMoveEffect_Player.json");
	playerBulletEffectConfig_.explosionEffectFilePath.push_back("DamageSpark2.json");
	enemyBulletEffectConfig_.trailEffectFilePath.push_back("BulletMoveEffect_Enemy.json");
	enemyBulletEffectConfig_.explosionEffectFilePath.push_back("DamageSpark2.json");

	playerMissileEffectConfig_.trailEffectFilePath.push_back("MissileSmoke.json");
	playerMissileEffectConfig_.explosionEffectFilePath.push_back("MissileExplosion_Player.json");
	enemyMissileEffectConfig_.trailEffectFilePath.push_back("MissileSmoke.json");
	enemyMissileEffectConfig_.explosionEffectFilePath.push_back("MissileExplosion_Enemy.json");

	playerBazookaBulletEffectConfig_.trailEffectFilePath.push_back("BazookaBulletTrail_Player.json");
	playerBazookaBulletEffectConfig_.explosionEffectFilePath.push_back("BazookaBulletExplosion_Player.json");
	playerBazookaBulletEffectConfig_.explosionEffectFilePath.push_back("BazookaBulletExplosion2.json");
	enemyBazookaBulletEffectConfig_.trailEffectFilePath.push_back("BazookaBulletTrail_Enemy.json");
	enemyBazookaBulletEffectConfig_.explosionEffectFilePath.push_back("BazookaBulletExplosion_Enemy.json");
	enemyBazookaBulletEffectConfig_.explosionEffectFilePath.push_back("BazookaBulletExplosion2.json");

	bulletFilePath_ = "Bullet.gltf";
	missileFilePath_ = "Missile.gltf";
	bazookaBulletFilePath_ = "BazookaBullet.gltf";
	playerBulletPool_ = std::make_unique<BulletPool>();
	playerBulletPool_->Initialize(object3dCommon_,size,bulletFilePath_, playerBulletEffectConfig_);
	enemyBulletPool_ = std::make_unique<BulletPool>();
	enemyBulletPool_->Initialize(object3dCommon_,size,bulletFilePath_, enemyBulletEffectConfig_);

	playerMissilePool_ = std::make_unique<MissilePool>();
	playerMissilePool_->Initialize(object3dCommon_,size * 2,missileFilePath_, playerMissileEffectConfig_);
	enemyMissilePool_ = std::make_unique<MissilePool>();
	enemyMissilePool_->Initialize(object3dCommon_,size * 2,missileFilePath_, enemyMissileEffectConfig_);

	playerBazookaBulletPool_ = std::make_unique<BulletPool>();
	playerBazookaBulletPool_->Initialize(object3dCommon_, size,bazookaBulletFilePath_, playerBazookaBulletEffectConfig_);
	enemyBazookaBulletPool_ = std::make_unique<BulletPool>();
	enemyBazookaBulletPool_->Initialize(object3dCommon_, size,bazookaBulletFilePath_, enemyBazookaBulletEffectConfig_);

}

//========================================================================================================
// 終了処理
//========================================================================================================

void BulletManager::Finalize() {

	playerMissilePool_->Finalize();
	playerBulletPool_->Finalize();
	playerBazookaBulletPool_->Finalize();
	enemyBulletPool_->Finalize();
	enemyMissilePool_->Finalize();
	enemyBazookaBulletPool_->Finalize();
	object3dCommon_ = nullptr;
}

//========================================================================================================
// 全弾の更新処理
//========================================================================================================

void BulletManager::Update() {
	// 弾の更新
	playerBulletPool_->UpdateAll();
	enemyBulletPool_->UpdateAll();
	// ミサイルの更新
	playerMissilePool_->UpdateAll();
	enemyMissilePool_->UpdateAll();
	// バズーカ弾の更新
	playerBazookaBulletPool_->UpdateAll();
	enemyBazookaBulletPool_->UpdateAll();
}



//========================================================================================================
// 各弾の描画処理
//========================================================================================================

void BulletManager::Draw() {

	// 弾の描画
	playerBulletPool_->DrawAll();
	enemyBulletPool_->DrawAll();
	// ミサイルの描画
	playerMissilePool_->DrawAll();
	enemyMissilePool_->DrawAll();
	// バズーカ弾の描画
	playerBazookaBulletPool_->DrawAll();
	enemyBazookaBulletPool_->DrawAll();
}

void BulletManager::DrawCollider() {

	playerBulletPool_->DrawAllCollider();
	playerMissilePool_->DrawAllCollider();
	playerBazookaBulletPool_->DrawAllCollider();

	enemyBulletPool_->DrawAllCollider();
	enemyMissilePool_->DrawAllCollider();
	enemyBazookaBulletPool_->DrawAllCollider();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,const Vector3& targetVel,float speed,float power,CharacterType type) {

	Bullet* bullet = nullptr;
	if (type == CharacterType::PLAYER_BULLET) {
		// プレイヤー弾の発射処理
		bullet = playerBulletPool_->GetObject();
	}
	else if (type == CharacterType::ENEMY_BULLET) {
		// エネミー弾の発射処理
		bullet = enemyBulletPool_->GetObject();
	}

	bullet->Create(weaponPos, targetPos,targetVel,speed,power,type);
}

void BulletManager::ShootBullet(const Vector3& weaponPos, const Vector3& direction,float speed, float power, CharacterType type) {

	Bullet* bullet = nullptr;
	if (type == CharacterType::PLAYER_BULLET) {
		// プレイヤー弾の発射処理
		bullet = playerBulletPool_->GetObject();
	}
	else if (type == CharacterType::ENEMY_BULLET) {
		// エネミー弾の発射処理
		bullet = enemyBulletPool_->GetObject();
	}
	bullet->Create(weaponPos, direction, speed, power, type);
}

//========================================================================================================
// ミサイルの発射処理
//========================================================================================================
void BulletManager::ShootMissile(BaseWeapon* ownerWeapon,VerticalMissileInfo vmInfo, float speed,float power, CharacterType type) {

	VerticalMissile* missile = nullptr;
	if (type == CharacterType::PLAYER_MISSILE) {
		// プレイヤーミサイルの発射処理
		missile = playerMissilePool_->GetObject();
	}
	else if (type == CharacterType::ENEMY_MISSILE) {
		// エネミーミサイルの発射処理
		missile = enemyMissilePool_->GetObject();
	}
	if (missile == nullptr) {
		return; // ミサイルが取得できなかった場合は何もしない
	}
	missile->Create(ownerWeapon,vmInfo, speed,power, type);
}

//========================================================================================================
// バズーカ弾の発射処理
//========================================================================================================
void BulletManager::ShootBazookaBullet(
	const Vector3& weaponPos,
	const Vector3& targetPos,
	const Vector3& targetVel,
	float speed, float power,
	CharacterType type) {

	Bullet* bazookaBullet = nullptr;
	if (type == CharacterType::PLAYER_BULLET) {
		// プレイヤーバズーカ弾の発射処理
		bazookaBullet = playerBazookaBulletPool_->GetObject();
	}
	else if (type == CharacterType::ENEMY_BULLET) {
		// エネミーバズーカ弾の発射処理
		bazookaBullet = enemyBazookaBulletPool_->GetObject();
	}
	bazookaBullet->Create(weaponPos, targetPos, targetVel, speed, power, type);
}

void BulletManager::ShootBazookaBullet(
	const Vector3& weaponPos,
	const Vector3& direction,
	float speed, float power,
	CharacterType type) {

	Bullet* bazookaBullet = nullptr;
	if (type == CharacterType::PLAYER_BULLET) {
		// プレイヤーバズーカ弾の発射処理
		bazookaBullet = playerBazookaBulletPool_->GetObject();
	}
	else if (type == CharacterType::ENEMY_BULLET) {
		// エネミーバズーカ弾の発射処理
		bazookaBullet = enemyBazookaBulletPool_->GetObject();
	}
	bazookaBullet->Create(weaponPos, direction, speed, power, type);
}

//========================================================================================================
// 全弾の取得
//========================================================================================================
std::vector<Bullet*> BulletManager::GetAllPlayerBullets() {
	
	std::vector<Bullet*> bullets;
	for (const auto& bullet : playerBulletPool_->GetPool()) {
		if (bullet->IsActive()) {
			bullets.push_back(bullet);
		}
	}
	return bullets;
}

std::vector<Bullet*> BulletManager::GetAllEnemyBullets() {
	std::vector<Bullet*> bullets;
	for (const auto& bullet : enemyBulletPool_->GetPool()) {
		if (bullet->IsActive()) {
			bullets.push_back(bullet);
		}
	}
	return bullets;
}

//========================================================================================================
// 全ミサイルの取得
//========================================================================================================
std::vector<VerticalMissile*> BulletManager::GetAllPlayerMissiles() {
	
	std::vector<VerticalMissile*> missiles;
	for (const auto& missile : playerMissilePool_->GetPool()) {
		if (missile->IsActive()) {
			missiles.push_back(missile);
		}
	}
	return missiles;
}

std::vector<VerticalMissile*> BulletManager::GetAllEnemyMissiles() {
	
	std::vector<VerticalMissile*> missiles;
	for (const auto& missile : enemyMissilePool_->GetPool()) {
		if (missile->IsActive()) {
			missiles.push_back(missile);
		}
	}
	return missiles;
}

//========================================================================================================
// 全バズーカ弾の取得
//========================================================================================================
std::vector<Bullet*> BulletManager::GetAllPlayerBazookaBullets() {
	
	std::vector<Bullet*> bazookaBullets;
	for (const auto& bullet : playerBazookaBulletPool_->GetPool()) {
		if (bullet->IsActive()) {
			bazookaBullets.push_back(bullet);
		}
	}
	return bazookaBullets;
}

std::vector<Bullet*> BulletManager::GetAllEnemyBazookaBullets() {
	
	std::vector<Bullet*> bazookaBullets;
	for (const auto& bullet : enemyBazookaBulletPool_->GetPool()) {
		if (bullet->IsActive()) {
			bazookaBullets.push_back(bullet);
		}
	}
	return bazookaBullets;
}