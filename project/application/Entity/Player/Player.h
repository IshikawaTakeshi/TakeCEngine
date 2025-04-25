#pragma once
#include "application/Entity/GameCharacter.h"
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

	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	Behavior behavior_ = Behavior::IDLE;

	bool isJumping_ = false;
	bool isDashing_ = false;
	bool onGround_ = false;

};

