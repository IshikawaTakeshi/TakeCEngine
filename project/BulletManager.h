#pragma once
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/Bullet/BulletPool.h"
#include "3d/Object3dCommon.h"
#include <string>

class BulletManager {
public:

	BulletManager() = default;
	~BulletManager() = default;

	void Initialize(Object3dCommon* object3dCommon,size_t size);
	void Finalize();
	void UpdateBullet();
	void DrawBullet();

	void ShootBullet(const Vector3& weaponPos,const Vector3& targetPos);

private:

	Object3dCommon* object3dCommon_ = nullptr;
	std::string bulletFilePath_ = nullptr;
	BulletPool bulletPool_;
};