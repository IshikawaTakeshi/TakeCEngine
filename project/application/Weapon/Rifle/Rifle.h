#pragma once
#include "Weapon/BaseWeapon.h"
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/Rifle/RifleInfo.h"
#include "application/Effect/MuzzleFlashEffect.h"

//============================================================================
// Rifle class
//============================================================================
class Rifle : public BaseWeapon {
public:
	Rifle() = default;
	~Rifle() = default;

	//===================================================================
	// functions
	//===================================================================

	//初期化処理
	void Initialize(Object3dCommon* object3dCommon,BulletManager* bulletManager) override;
	//更新処理
	void Update() override;
	//ImGuiの更新
	void UpdateImGui() override;
	//描画処理
	void Draw() override;
	// 攻撃処理
	void Attack() override;
	// チャージ攻撃処理
	void Charge(float deltaTime) override;
	// チャージ攻撃実行
	void ChargeAttack() override;

private:

	//連射情報
	RifleInfo rifleInfo_{};
	BurstShotState burstShotState_{};

	//マズルフラッシュエフェクト
	std::unique_ptr<MuzzleFlashEffect> muzzleFlashEffect_ = nullptr;
};