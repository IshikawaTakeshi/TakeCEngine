#pragma once
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/Bullet/VerticalMissile.h"
#include "Weapon/Bullet/BulletPool.h"
#include "Weapon/Bullet/MissilePool.h"
#include "3d/Object3dCommon.h"
#include <string>
#include <memory>
#include <vector>

class BulletManager {
public:

	BulletManager() = default;
	~BulletManager() = default;

	void Initialize(Object3dCommon* object3dCommon,size_t size);
	void Finalize();
	void UpdateBullet();
	void UpdateMissile();
	void Update();
	void Draw();
	void DrawCollider();

	void ShootBullet(const Vector3& weaponPos, const Vector3& targetPos, const float& speed, CharacterType type);
	void ShootMissile(BaseWeapon* ownerWeapon, const float& speed, CharacterType type);

	std::vector<Bullet*> GetAllBullets();
	std::vector<VerticalMissile*> GetAllMissiles();

private:

	Object3dCommon* object3dCommon_ = nullptr;
	std::string bulletFilePath_;
	std::string missileFilePath_;
	std::unique_ptr<BulletPool> bulletPool_;
	std::unique_ptr<MissilePool> missilePool_;
};