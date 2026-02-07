#pragma once
#include "application/Weapon/BaseWeapon.h"
#include "application/Weapon/Bullet/VerticalMissileInfo.h"
#include "application/Weapon/Bullet/BulletEffectConfig.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include "Bullet.h"

//============================================================================
// VerticalMissile class
//============================================================================
class VerticalMissile : public GameCharacter {
public:

	//ミサイルのフェーズ
	enum class VerticalMissilePhase {
		ASCENDING, // 上昇中
		HOMING,    // 目標に向かっている
		EXPLODING, // 爆発中
	};

	VerticalMissile() = default;
	~VerticalMissile() = default;
	
	//========================================================================
	// functions
	//========================================================================

	// 初期化
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void InitializeEffect(const BulletEffectConfig& effectConfig);
	// 更新処理
	void Update() override;
	// ImGuiの更新
	void UpdateImGui();
	// 描画処理
	void Draw() override;
	// コライダー描画
	void DrawCollider() override;
	// 衝突時の処理
	void OnCollisionAction(GameCharacter* other) override;
	// ミサイルの生成
	void Create(BaseWeapon* ownerWeapon,VerticalMissileInfo vmInfo, float speed,float damage, CharacterType type);

public:

	//===========================================================================
	// getter
	//===========================================================================

	const QuaternionTransform& GetTransform() const;
	bool IsActive() const;
	const Vector3& GetVelocity() const;
	const Vector3& GetTargetPos() const;
	float GetDamage() const;
	float GetSpeed() const;
	float GetBulletRadius() const;
	float GetLifeTime() const;
	const VerticalMissilePhase& GetPhase() const;


	//============================================================================
	// setter
	//============================================================================

	void SetIsActive(bool isActive) { isActive_ = isActive; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetTargetPos(const Vector3& targetPos) { targetPos_ = targetPos; }
	void SetSpeed(float speed) { speed_ = speed; }
	void SetLifeTime(float lifeTime) { lifeTime_ = lifeTime; }
	void SetTransform(const QuaternionTransform& transform) { transform_ = transform; }

private:

	BaseWeapon* ownerWeapon_ = nullptr; // 所有者の武器
	QuaternionTransform transform_{};
	float deltaTime_ = 0.0f;
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	Vector3 targetPos_ = { 0.0f,0.0f,0.0f };
	Vector3 direction_ = { 0.0f,0.0f,0.0f };
	float speed_ = 0.0f; // 弾速
	float damage_ = 0.0f; // 攻撃力
	bool isActive_ = false; // 生存フラグ
	float lifeTime_ = 0.0f; // 寿命時間
	float bulletRadius_ = 1.0f; //弾の半径

	PointLightData pointLightData_;
	uint32_t pointLightIndex_ = 0;

	BulletEffectConfig effectConfig_;

	//パーティクルエミッター
	std::vector<std::unique_ptr<ParticleEmitter>> trailEmitter_;
	std::vector<std::unique_ptr<ParticleEmitter>> explosionEmitter_;

	//ミサイルのフェーズ
	VerticalMissilePhase phase_ = VerticalMissilePhase::ASCENDING;

	// ホーミング特性
	VerticalMissileInfo vmInfo_{};
};