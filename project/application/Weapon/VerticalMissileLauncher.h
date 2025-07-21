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

	// チャージ処理
	virtual void Charge(float deltaTime) override;
	// チャージ攻撃実行
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

	bool isBursting_ = false;                // 3連射中かどうか
	int burstCount_ = 0;                     // 3連射のカウント
	float burstInterval_ = 0.0f;             // 3連射の間隔
	const int kMaxBurstCount = 3;            // 3連射の最大カウント
	const float kBurstAttackInterval = 0.1f; // 3連射時の攻撃間隔
};

