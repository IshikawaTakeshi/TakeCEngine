#pragma once
#include "Weapon/BaseWeapon.h"
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/BurstShotInfo.h"

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
	void Initialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& filePath) override;
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

	//----- setter ---------------------------

	// 武器タイプの取得
	void SetOwnerObject(GameCharacter* owner) override;
	//攻撃対象の座標を設定
	void SetTarget(const Vector3& targetPos) override { targetPos_ = targetPos; }

	//チャージ攻撃可能か
	bool IsChargeAttack() const override;
	//移動撃ち可能か
	bool IsMoveShootable() const override;
		//停止撃ち専用か
	bool IsStopShootOnly() const override;

private:

	//連射情報
	BurstShotInfo burstShotInfo_{};
	BurstShotState burstShotState_{};
};