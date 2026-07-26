#pragma once
#include "application/Weapon/BaseWeapon.h"
#include "application/Weapon/Launcher/VerticalMissileLauncherInfo.h"

//============================================================================
// VerticalMissileLauncher class
//============================================================================
/// <summary>
/// 垂直発射ミサイルの生成、照準、発射を制御する武器クラスです。
/// </summary>
class VerticalMissileLauncher : public BaseWeapon {
public:

	VerticalMissileLauncher() = default;
	~VerticalMissileLauncher() = default;

	//=====================================================================
	// functions
	//=====================================================================

	// 初期化処理
	void Initialize(TakeC::Object3dCommon* object3dCommon, BulletManager* bulletManager) override;
	// 更新処理
	void Update() override;
	// ImGuiの更新
	void UpdateImGui() override;
	// 描画処理
	void Draw() override;
	// 攻撃処理
	void Attack() override;

	// チャージ処理
	virtual void Charge(float deltaTime) override;
	// チャージ攻撃実行
	virtual void ChargeAttack() override;

private:

	VerticalMissileLauncherInfo vmLauncherInfo_{};
	BurstShotState burstShotState_{};
};