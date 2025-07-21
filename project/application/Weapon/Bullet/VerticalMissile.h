#pragma once
#include "application/Weapon/BaseWeapon.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Particle/ParticleEmitter.h"

class VerticalMissile : public GameCharacter {
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
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;
	void Create(BaseWeapon* ownerWeapon, const float& speed, CharacterType type);

public:

	EulerTransform GetTransform() const { return transform_; }
	bool GetIsActive() { return isActive_; }

	void SetIsActive(bool isActive) { isActive_ = isActive; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetTargetPos(const Vector3& targetPos) { targetPos_ = targetPos; }
	void SetSpeed(float speed) { speed_ = speed; }
	void SetLifeTime(float lifeTime) { lifeTime_ = lifeTime; }
	void SetTransform(const EulerTransform& transform) { transform_ = transform; }

private:

	BaseWeapon* ownerWeapon_ = nullptr; // 所有者の武器
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

	//ミサイルのフェーズ
	VerticalMissilePhase phase_ = VerticalMissilePhase::ASCENDING;

	//ミサイルの上昇高度
	float altitude_ = 0.0f;
	//撃ちだされてからの上昇値
	const float kMaxAltitude_ = 100.0f;

	//ミサイルの上昇速度
	const float kAscendSpeed_ = 140.0f;
	//ミサイルの爆発半径
	const float kExplosionRadius_ = 5.0f;
	//ホーミングの度合い
	float homingRate_ = 0.1f;
};

