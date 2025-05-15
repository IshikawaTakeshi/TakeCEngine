#pragma once
#include "Weapon/IWeapon.h"
#include "Weapon/Bullet.h"

class Rifle : public IWeapon {
public:
	Rifle() = default;
	~Rifle();

	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void Draw() override;
	void Attack() override;
	int32_t GetWeaponType() const override { return weaponType_; }
	void SetOwner(int32_t owner) override { owner_ = owner; }

private:
	
};

