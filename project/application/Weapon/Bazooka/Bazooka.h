#pragma once
#include "Weapon/BaseWeapon.h"
#include "Weapon/Bullet/BulletManager.h"

//============================================================================
// Bazooka class
//============================================================================
class Bazooka : public BaseWeapon {
public:
	Bazooka() = default;
	~Bazooka() override = default;
	
	//========================================================================
	// functions
	//========================================================================
	
	// 武器の初期化
	void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) override;
	// 武器の更新
	void Update() override;
	// ImGuiの更新
	void UpdateImGui() override;
	// 武器の描画
	void Draw() override;
	// 武器の攻撃
	void Attack() override;

};