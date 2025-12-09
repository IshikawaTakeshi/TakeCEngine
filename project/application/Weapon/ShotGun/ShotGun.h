#pragma once
#include "application/Weapon/BaseWeapon.h"
#include "application/Weapon/Bullet/Bullet.h"
#include "application/Weapon/ShotGun/ShotGunInfo.h"

//============================================================================
// ShotGun class
//============================================================================

class ShotGun : public BaseWeapon {
public:

	//========================================================================
	// functions
	//========================================================================

	// コンストラクタ・デストラクタ
	ShotGun() = default;
	~ShotGun() override = default;

	
	void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;
	void Attack() override;

private:

	// ショットガンの情報
	ShotGunInfo shotGunInfo_;
};