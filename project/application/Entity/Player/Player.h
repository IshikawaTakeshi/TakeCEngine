#pragma once
#include "engine/Entity/GameCharacter.h"
#include "engine/io/Gamepad.h"
#include "engine/camera/Camera.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include <optional>
#include <memory>

#include "application/Weapon/BaseWeapon.h"
#include "application/Weapon/WeaponType.h"
#include "application/Weapon/Bullet/BulletManager.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include "application/Provider/PlayerInputProvider.h"
#include "application/Entity/Behavior/BehaviorManager.h"


class Player : public GameCharacter {
public:
	Player() = default;
	~Player() override;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager);

public:

	//==============================================================================
	// getter
	//==============================================================================

	BaseWeapon* GetWeapon(int index) const;

	std::vector<std::unique_ptr<BaseWeapon>>& GetWeapons();

	const GameCharacterContext& GetCharacterInfo() const { return characterInfo_; }
	GameCharacterContext& GetCharacterInfo() { return characterInfo_; }

	Camera* GetCamera() const { return camera_; }

	//移動方向ベクトルの取得
	const Vector3& GetMoveDirection() const { return characterInfo_.moveDirection; }
	//移動ベクトルの取得
	const Vector3& GetVelocity() const { return characterInfo_.velocity; }
	//transformの取得
	const QuaternionTransform& GetTransform() const { return characterInfo_.transform; }

	//体力の取得
	float GetHealth() const { return characterInfo_.health; }
	//最大体力の取得
	const float GetMaxHealth() const { return characterInfo_.maxHealth; }
	//フォーカス対象の座標を取得
	const Vector3& GetFocusTargetPos() const { return focusTargetPos_; }

	//エネルギーの取得
	float GetEnergy() const { return characterInfo_.energyInfo.energy; }
	//最大エネルギーの取得
	float GetMaxEnergy() const { return characterInfo_.energyInfo.maxEnergy; }
	//エネルギーの回復速度の取得
	float GetEnergyRegenRate() const { return characterInfo_.energyInfo.recoveryRate; }

	bool GetIsOverHeated() const { return characterInfo_.overHeatInfo.isOverheated; }

	bool GetIsAlive() const { return characterInfo_.isAlive; }

	//================================================================================
	// setter
	//================================================================================

	//プレイヤーの体力を設定
	void SetHealth(float health) { characterInfo_.health = health; }
	//フォーカス対象の座標を設定
	void SetFocusTargetPos(const Vector3& targetPos) { focusTargetPos_ = targetPos; }

private:

	void InitRunning();
	void InitJump();
	void InitDash();
	void InitChargeShoot();
	void InitChargeShootStun();
	void InitStepBoost();
	void InitFloating();
	void InitDead();

	void UpdateRunning();
	void UpdateAttack();
	void UpdateDamage();
	void UpdateJump();
	void UpdateDash();
	void UpdateChargeShoot();
	void UpdateChargeShootStun();
	void UpdateStepBoost();
	void UpdateFloating();
	void UpdateDead();

	// ステップブーストのBehavior切り替え処理
	void TriggerStepBoost();
	//武器一つ当たりの攻撃処理
	void WeaponAttack(int weaponIndex, GamepadButtonType buttonType);
	void WeaponChargeAttack(int weaponIndex);

	//エネルギーの更新
	void UpdateEnergy();

private:

	//カメラ
	Camera* camera_ = nullptr;
	//状態管理マネージャ
	std::unique_ptr<BehaviorManager> behaviorManager_ = nullptr;
	//プレイヤー入力プロバイダ
	std::unique_ptr<PlayerInputProvider> inputProvider_ = nullptr;

	//プレイヤーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;
	//チャージ撃ちをする武器ユニット
	std::vector<bool> chargeShootableUnits_;


	//背部のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> backEmitter_ = nullptr;
	// プレイヤーの情報
	GameCharacterContext characterInfo_;
	// フレーム時間
	float deltaTime_ = 0.0f; 
	float gravity_ = 9.8f;         // 重力の強さ
	//補足対象の座標
	Vector3 focusTargetPos_ = { 0.0f,100.0f,0.0f };

	
	float chargeShootDuration_ = 1.0f; // 停止撃ちの持続時間
	float chargeShootTimer_ = 0.0f; //停止撃ちまでの残り猶予時間
};