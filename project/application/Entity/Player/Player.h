#pragma once
#include "application/Entity/GameCharacter.h"
#include "camera/Camera.h"
#include <optional>

class Player : public GameCharacter {
public:
	Player() = default;
	~Player() override = default;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

private:

	enum class Behavior {
		IDLE,
		RUNNING,
		JUMP,
		DASH,
		CHARGEATTACK, //
		HEAVYDAMAGE,
	};

	void InitRunning();
	void InitJump();
	void InitDash();

	void UpdateRunning();
	void UpdateAttack();
	void UpdateDamage();
	void UpdateJump();
	void UpdateDash();

private:

	//カメラ
	Camera* camera_ = nullptr;

	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	Behavior behavior_ = Behavior::IDLE;

	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };

	EulerTransform transform_ = { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, {0.0f,0.0f,0.0f} };

	const float moveSpeed_ = 4.0f;
	const float kMaxMoveSpeed_ = 10.0f;

	float deltaTime_ = 0.0f;

	bool isJumping_ = false;
	bool isDashing_ = false;
	bool onGround_ = false;

};

