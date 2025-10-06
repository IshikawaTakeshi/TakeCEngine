#pragma once
#include "engine/Entity/GameCharacter.h"
#include "engine/camera/Camera.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include <optional>
#include <chrono>
#include <random>

#include "application/Weapon/Bullet/BulletManager.h"
#include "application/Weapon/BaseWeapon.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include "application/Provider/EnemyInputProvider.h"
#include "application/Entity/Behavior/BehaviorManager.h"
#include "application/Entity/Enemy/AIBrainSystem.h"
#include "application/Entity/Enemy/BulletSensor.h"

class Enemy : public GameCharacter {

public:
	Enemy() = default;
	~Enemy() override;
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

	BulletSensor* GetBulletSensor() const { return bulletSensor_.get(); }

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
	const Vector3& GetFocusTargetPos() const { return characterInfo_.focusTargetPos; }

	//エネルギーの取得
	float GetEnergy() const { return characterInfo_.energyInfo.energy; }
	//最大エネルギーの取得
	float GetMaxEnergy() const { return characterInfo_.energyInfo.maxEnergy; }
	//エネルギーの回復速度の取得
	float GetEnergyRegenRate() const { return characterInfo_.energyInfo.recoveryRate; }

	bool GetIsOverHeated() const { return characterInfo_.overHeatInfo.isOverheated; }

	bool GetIsAlive() const { return characterInfo_.isAlive; }

	float GetOrbitAngle() const { return orbitAngle_; }
	float GetOrbitRadius() const { return orbitRadius_; }
	float GetOrbitSpeed() const { return orbitSpeed_; }

	const Vector3& GetToOrbitPos() const { return toOrbitPos_; }

	//================================================================================
	// setter
	//================================================================================

	//プレイヤーの体力を設定
	void SetHealth(float health) { characterInfo_.health = health; }
	//フォーカス対象の座標を設定
	void SetFocusTargetPos(const Vector3& targetPos) { characterInfo_.focusTargetPos = targetPos; }

	void SetVelocity(const Vector3& velocity) { characterInfo_.velocity = velocity; }
	void SetOrbitAngle(float angle) { orbitAngle_ = angle; }
	void SetOrbitSpeed(float speed) { orbitSpeed_ = speed; }

private:

	void UpdateAttack();
	void WeaponAttack(int weaponIndex);

	//エネルギーの更新
	void UpdateEnergy();

private:
	// 攻撃開始判定
	//bool ShouldStartAttack(int weaponIndex);
	// チャージ攻撃実行判定
	bool ShouldReleaseAttack(int weaponIndex);

private:

	
	std::unique_ptr<AIBrainSystem> aiBrainSystem_ = nullptr;
	//接近してくる弾に反応するためのコライダー
	std::unique_ptr<BulletSensor> bulletSensor_ = nullptr;

	//状態管理マネージャ
	std::unique_ptr<BehaviorManager> behaviorManager_ = nullptr;
	//プレイヤー入力プロバイダ
	std::unique_ptr<EnemyInputProvider> inputProvider_ = nullptr;

	//プレイヤーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;
	//チャージ撃ちをする武器ユニット
	std::vector<bool> chargeShootableUnits_;

	//背部のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> backEmitter_ = nullptr;
	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;
	// プレイヤーの情報
	GameCharacterContext characterInfo_;
	// フレーム時間
	float deltaTime_ = 0.0f; 
	float gravity_ = 9.8f;         // 重力の強さ
	
	float chargeShootDuration_ = 1.0f; // 停止撃ちの持続時間
	float chargeShootTimer_ = 0.0f; //停止撃ちまでの残り猶予時間

	float damageEffectTime_ = 0.0f; // ダメージエフェクトの時間

	//攻撃確率(0~100)
	const float attackProbability_ = 10.0f;
	//ジャンプ確率(0~100)
	const float jumpProbability_ = 1.0f;
	//ターゲットの周りを周回するための変数
	float orbitAngle_ = 0.0f;              // 周回角度
	float orbitRadius_ = 60.0f;             // 周回半径（ターゲットとの距離）
	float orbitSpeed_ = 1.0f;              // 角速度（周る速さ）
	Vector3 toOrbitPos_ = { 0.0f,0.0f,0.0f }; // 周回する座標
};