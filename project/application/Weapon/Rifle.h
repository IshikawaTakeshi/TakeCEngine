#pragma once
#include "Weapon/BaseWeapon.h"
#include "Weapon/Bullet/Bullet.h"

class Rifle : public BaseWeapon {
public:
	Rifle() = default;
	~Rifle();

	void Initialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;
	void Attack() override;
	const WeaponType& GetWeaponType() const override { return weaponType_; }
	void SetOwnerObject(GameCharacter* owner) override;
	void SetTarget(const Vector3& targetPos) override { targetPos_ = targetPos; }

private:

	const float kAttackInterval = 0.25f; // 攻撃間隔定数
	
};

