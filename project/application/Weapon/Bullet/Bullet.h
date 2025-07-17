#pragma once
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Entity/GameCharacter.h"
#include "3d/Particle/ParticleEmitter.h"
#include <cstdint>
#include <string>
#include <memory>

enum class BulletType {
	NONE,
	BULLET,
	EXPLOSION,
};

class Bullet : public GameCharacter {
public:
	Bullet() = default;
	~Bullet() = default;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath)override;
	void Update() override;
	virtual void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	virtual void Create(const Vector3& weaponPos,const Vector3& targetPos,const float& speed,CharacterType type);

public:

	virtual EulerTransform GetTransform() const { return transform_; }
	virtual bool GetIsActive() { return isActive_; }

public:

	virtual void SetIsActive(bool isActive) { isActive_ = isActive; }
	virtual void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	virtual void SetSpeed(float speed) { speed_ = speed; }
	virtual void SetLifeTime(float lifeTime) { lifeTime_ = lifeTime; }
	virtual void SetTransform(const EulerTransform& transform) { transform_ = transform; }

protected:

	EulerTransform transform_{};
	float deltaTime_ = 0.0f;
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	Vector3 targetPos_ = { 0.0f,0.0f,0.0f };
	Vector3 direction_ = { 0.0f,0.0f,0.0f };
	float speed_ = 0.0f;
	bool isActive_ = false;
	float lifeTime_ = 0.0f;
	float bulletradius_ = 1.0f; //弾の半径

	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;
};

