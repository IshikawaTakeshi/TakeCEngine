#pragma once
#include "application/Weapon/BaseWeapon.h"
#include "application/Weapon/Bullet/Bullet.h"
#include "application/Weapon/ShotGun/ShotGunInfo.h"

//============================================================================
// ShotGun class
//============================================================================

/// <summary>
/// 複数の弾を拡散発射するショットガン武器を制御するクラスです。
/// </summary>
class ShotGun : public BaseWeapon {
public:

	//========================================================================
	// functions
	//========================================================================

	// コンストラクタ・デストラクタ
	ShotGun() = default;
	~ShotGun() override = default;

	
	void Initialize(TakeC::Object3dCommon* object3dCommon, BulletManager* bulletManager) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;
	void Attack() override;

private:

	// ショットガンの情報
	ShotGunInfo shotGunInfo_;
};