#pragma once
#include "application/Weapon/BaseWeapon.h"

//============================================================================
// MachineGun class
//============================================================================
class MachineGun : public BaseWeapon {
public:

	//========================================================================
	// function
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	MachineGun() = default;
	~MachineGun() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="object3dCommon"></param>
	/// <param name="bulletManager"></param>
	/// <param name="filePath"></param>
	void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 攻撃処理
	/// </summary>
	void Attack() override;
};

