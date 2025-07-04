#pragma once
#include "Weapon/BaseWeapon.h"
#include "Weapon/Bullet/BulletManager.h"

class Bazooka : public BaseWeapon {
public:
	Bazooka() = default;
	~Bazooka() override = default;
	// 武器の初期化
	void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) override;
	// 武器の更新
	void Update() override;
	// ImGuiの更新
	void UpdateImGui() override;
	// 武器の描画
	void Draw() override;
	// 武器の攻撃
	void Attack() override;
	// 武器タイプの取得
	const WeaponType& GetWeaponType() const override { return weaponType_; }
	// 所有者の設定
	void SetOwnerObject(GameCharacter* owner) override;
	// 攻撃対象の座標を設定
	void SetTarget(const Vector3& targetPos) override { targetPos_ = targetPos; }

public:

	const float kAttackInterval = 2.0f; // 攻撃間隔定数

};

