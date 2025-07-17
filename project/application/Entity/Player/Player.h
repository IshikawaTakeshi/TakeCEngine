#pragma once
#include "engine/Entity/GameCharacter.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/WeaponType.h"
#include "camera/Camera.h"
#include "Particle/ParticleEmitter.h"
#include <optional>

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

	void SetFocusTargetPos(const Vector3& targetPos) { focusTargetPos_ = targetPos; }

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

	//カメラ
	Camera* camera_ = nullptr;
	//状態遷移リクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//プレイヤーの状態
	Behavior behavior_ = Behavior::IDLE;
	Behavior prevBehavior_ = Behavior::IDLE;
	//プレイヤーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;

	//背部のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> backEmitter_ = nullptr;

	//補足対象の座標
	Vector3 focusTargetPos_ = { 0.0f,100.0f,0.0f };

	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };

	QuaternionTransform transform_;

	const float moveSpeed_ = 200.0f;
	const float kMaxMoveSpeed_ = 120.0f;

	//QBInfo
	Vector3 stepBoostDirection_ = { 0.0f,0.0f,0.0f }; // ステップブーストの方向
	const float stepBoostSpeed_ = 230.0f;             // ステップブーストの速度
	const float stepBoostDuration_ = 0.2f;            // ステップブーストの持続時間
	float stepBoostTimer_ = 0.0f;                     // ステップブーストのタイマー
	//インターバル用
	const float stepBoostInterval_ = 0.2f; // ステップブーストのインターバル
	float stepBoostIntervalTimer_ = 0.0f; // ステップブーストのインターバルタイマー

	//JumInfo
	//const float jumpHeight_ = 80.0f; // ジャンプの高さ
	const float jumpSpeed_ = 50.0f; // ジャンプの速度
	float jumpTimer_ = 0.0f; // ジャンプのタイマー
	const float maxJumpTime_ = 0.5f; // ジャンプの最大時間
	const float jumpDeceleration_ = 40.0f; // ジャンプ中の減速率


	const float gravity_ = 9.8f; // 重力の強さ
	//落下速度
	float fallSpeed_ = 40.0f; // 落下速度

	// チャージ攻撃後硬直用の変数
	float chargeAttackStunTimer_ = 0.0f;          //チャージ攻撃後の硬直時間
	const float chargeAttackStunDuration_ = 0.5f; // チャージ攻撃後の硬直時間
	float deltaTime_ = 0.0f;

	bool isJumping_ = false;
	bool isDashing_ = false;
	bool onGround_ = false;
};