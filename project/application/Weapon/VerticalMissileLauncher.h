#pragma once
#include "application/Weapon/BaseWeapon.h"

class VerticalMissileLauncher : public BaseWeapon {
public:

	VerticalMissileLauncher() = default;
	~VerticalMissileLauncher() = default;

	void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;
	void Attack() override;

	virtual void Charge(float deltaTime) override;
	virtual void ChargeAttack() override;

	// 武器タイプの取得
	const WeaponType& GetWeaponType() const override { return weaponType_; }
	void SetOwnerObject(GameCharacter* owner) override;
	void SetTarget(const Vector3& targetPos) override { targetPos_ = targetPos; }

	//チャージ攻撃可能か
	bool IsChargeAttack() const override;
	//移動撃ち可能か
	bool IsMoveShootable() const override;
	//停止撃ち専用か
	bool IsStopShootOnly() const override;

private:

	const float kAttackInterval = 0.1f; // 攻撃間隔定数
};

