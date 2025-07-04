#pragma once
#include "application/Entity/GameCharacter.h"
#include "Weapon/BaseWeapon.h"
#include "camera/Camera.h"
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

	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& weaponFilePath);

public:

	void SetFocusTargetPos(const Vector3& targetPos) { focusTargetPos_ = targetPos; }

private:

	enum class Behavior {
		IDLE,
		RUNNING,
		JUMP,
		DASH,
		CHARGEATTACK, //
		HEAVYDAMAGE,
		STEPBOOST,
	};

	void InitRunning();
	void InitJump();
	void InitDash();
	void InitStepBoost();

	void UpdateRunning();
	void UpdateAttack();
	void UpdateDamage();
	void UpdateJump();
	void UpdateDash();
	void UpdateStepBoost();

private:

	//カメラ
	Camera* camera_ = nullptr;
	//状態遷移リクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//プレイヤーの状態
	Behavior behavior_ = Behavior::IDLE;
	//プレイヤーの武器
	std::unique_ptr<BaseWeapon> weapon_ = nullptr;

	//補足対象の座標
	Vector3 focusTargetPos_ = { 0.0f,0.0f,0.0f };

	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };

	QuaternionTransform transform_;

	const float moveSpeed_ = 50.0f;
	const float kMaxMoveSpeed_ = 50.0f;

	//QBInfo
	Vector3 stepBoostDirection_ = { 0.0f,0.0f,0.0f }; // ステップブーストの方向
	const float stepBoostSpeed_ = 200.0f;
	const float stepBoostDuration_ = 0.3f; // ステップブーストの持続時間
	float stepBoostTimer_ = 0.0f; // ステップブーストのタイマー

	float deltaTime_ = 0.0f;

	bool isJumping_ = false;
	bool isDashing_ = false;
	bool onGround_ = false;

};

