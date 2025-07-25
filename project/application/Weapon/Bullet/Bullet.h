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
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void Create(const Vector3& weaponPos,const Vector3& targetPos,float speed,float damage,CharacterType type);

public:

	//===========================================================================
	// getter
	//===========================================================================

	//transformの取得
	const EulerTransform& GetTransform() const;
	//生存フラグの取得
	bool GetIsActive();

	//速度の取得
	const Vector3& GetVelocity() const;
	//ターゲット座標の取得
	const Vector3& GetTargetPos() const;

	//攻撃力の取得
	float GetDamage() const;
	//弾速の取得
	float GetSpeed() const;
	//弾の半径の取得
	float GetBulletRadius() const;
	//寿命時間の取得
	float GetLifeTime() const;

public:

	//trasformの設定
	void SetTransform(const EulerTransform& transform) { transform_ = transform; }
	//生存フラグの設定
	void SetIsActive(bool isActive);
	//速度の設定
	void SetVelocity(const Vector3& velocity);
	//ターゲット座標の設定
	void SetTargetPos(const Vector3& targetPos);
	//弾速の設定
	void SetSpeed(float speed);
	//攻撃力の設定
	void SetDamage(float damage);
	//弾の半径を設定
	void SetBulletRadius(float radius);
	//寿命時間の設定
	void SetLifeTime(float lifeTime) { lifeTime_ = lifeTime; }

private:

	EulerTransform transform_{};
	float deltaTime_ = 0.0f;
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	Vector3 targetPos_ = { 0.0f,0.0f,0.0f };
	Vector3 direction_ = { 0.0f,0.0f,0.0f };
	//攻撃力
	float damage_ = 0.0f;
	//弾速
	float speed_ = 0.0f;
	bool isActive_ = false;
	//寿命時間
	float lifeTime_ = 0.0f;
	//弾の半径
	float bulletradius_ = 1.0f;

	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;
};

