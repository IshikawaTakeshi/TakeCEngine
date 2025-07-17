#pragma once
#include "application/Weapon/Bullet/Bullet.h"

class VerticalMissile : public Bullet {
public:

	enum class VerticalMissilePhase {
		ASCENDING, // 上昇中
		HOMING,    // 目標に向かっている
		EXPLODING, // 爆発中
	};

	VerticalMissile() = default;
	~VerticalMissile() override = default;

	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;
	void Create(const Vector3& weaponPos, const Vector3& targetPos, const float& speed, CharacterType type) override;

public:

	EulerTransform GetTransform() const override { return transform_; }
	bool GetIsActive() override { return isActive_; }

	void SetIsActive(bool isActive) override { isActive_ = isActive; }
	void SetVelocity(const Vector3& velocity) override { velocity_ = velocity; }
	void SetSpeed(float speed) override { speed_ = speed; }
	void SetLifeTime(float lifeTime) override { lifeTime_ = lifeTime; }
	void SetTransform(const EulerTransform& transform) override { transform_ = transform; }

private:

	//ミサイルのフェーズ
	VerticalMissilePhase phase_ = VerticalMissilePhase::ASCENDING;

	//ミサイルの最大上昇高度
	const float kMaxAltitude = 100.0f;
	//ミサイルの上昇速度
	const float kAscendSpeed = 50.0f;
	//ミサイルの爆発半径
	const float kExplosionRadius = 5.0f;
	//ホーミングの度合い
	float homingRate = 0.1f;
};

