#pragma once
#include "Entity/GameCharacter.h"
#include "Weapon/BaseWeapon.h"
#include "camera/Camera.h"
#include "3d/Particle/ParticleEmitter.h"
#include <optional>

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

	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& weaponFilePath);

private:

	enum class Behavior {
		IDLE,
		RUNNING,
		JUMP,
		DASH,
		CHARGESHOOT, //チャージ攻撃中
		CHARGESHOOT_STUN, // チャージショット後の硬直状態
		HEAVYDAMAGE,
		STEPBOOST,
		FLOATING,
	};

	void InitRunning();
	void InitJump();
	void InitDash();
	void InitChargeShoot();
	void InitChargeShootStun();
	void InitStepBoost();
	void InitFloating();

	void UpdateRunning();
	void UpdateAttack();
	void UpdateDamage();
	void UpdateJump();
	void UpdateDash();
	void UpdateChargeShoot();
	void UpdateChargeShootStun();
	void UpdateStepBoost();
	void UpdateFloating();

	// ステップブーストのBehavior切り替え処理
	void TriggerStepBoost();

private:

	//状態遷移リクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//プレイヤーの状態
	Behavior behavior_ = Behavior::IDLE;
	//プレイヤーの武器
	std::unique_ptr<BaseWeapon> weapon_ = nullptr;
	//particleEmitter
	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;

	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };
	//補足対象の座標
	Vector3 focusTargetPos_ = { 0.0f,0.0f,0.0f };

	QuaternionTransform transform_ = { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,0.0f} };

	const float moveSpeed_ = 200.0f;
	const float kMaxMoveSpeed_ = 50.0f;

	//QBInfo
	Vector3 stepBoostDirection_ = { 0.0f,0.0f,0.0f }; // ステップブーストの方向
	const float stepBoostSpeed_ = 230.0f;
	const float stepBoostDuration_ = 0.2f; // ステップブーストの持続時間
	float stepBoostTimer_ = 0.0f; // ステップブーストのタイマー
	//インターバル用
	const float stepBoostInterval_ = 0.2f; // ステップブーストのインターバル
	float stepBoostIntervalTimer_ = 0.0f; // ステップブーストのインターバルタイマー

	//JumInfo
	const float jumpHeight_ = 80.0f; // ジャンプの高さ
	const float jumpSpeed_ = 50.0f; // ジャンプの速度
	float jumpTimer_ = 0.0f; // ジャンプのタイマー
	const float maxJumpTime_ = 0.5f; // ジャンプの最大時間
	const float gravity_ = 50.0f; // 重力の強さ

	//チャージ攻撃後の硬直時間
	float chargeAttackStunTimer_ = 0.0f;
	const float chargeAttackStunDuration_ = 0.5f; // チャージ攻撃後の硬直時間

	float deltaTime_ = 0.0f;

	uint32_t hitPoint_ = 100; //体力

	//ダメージを受けた時のエフェクト適用時間
	float damageEffectTime_ = 0.0f;

	bool isJumping_ = false;
	bool isDashing_ = false;
	bool onGround_ = false;
	bool isDamaged_ = false; //ダメージを受けたかどうか
	
};

