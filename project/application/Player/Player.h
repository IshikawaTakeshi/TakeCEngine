#pragma once
#include "Audio.h"
#include "Sprite.h"
#include "Camera.h"
#include "Collision/Collider.h"
#include "ParticleEmitter.h"
#include <list>

class Sprite;
class PlayerBullet;

/// <summary>
/// プレイヤー
/// </summary>
class Player : public Collider {
public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	Player() = default;
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// UIの描画処理
	/// </summary>
	void DrawUI();

	/// <summary>
	/// 移動入力処理
	/// </summary>
	void Move();
	
	/// <summary>
	/// ImGuiの更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 衝突を検出したら呼び出されるコールバック関数
	/// </summary>
	void OnCollision([[maybe_unused]] Collider* other) override;

	void UpdateInvincibleTime();

public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		getter
	////////////////////////////////////////////////////////////////////////////////////////


	/// <summary>
	/// worldTransformの取得
	/// </summary>
	//const WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// velocityの取得
	/// </summary>
	const Vector3& GetVelocity() { return velocity_; }

	/// <summary>
	/// ワールド座標の取得
	/// </summary>
	Vector3 GetWorldPos() override;

	/// <summary>
	/// 3Dレティクルのワールド座標の取得
	/// </summary>
	Vector3 GetWorldPos3DReticle();

	const float GetRadius() { return radius_; }

	/// <summary>
	/// 生存フラグの取得
	/// </summary>
	bool GetIsAlive() { return isAlive_; }

	/// <summary>
	///	弾リストの取得
	/// </summary>
	const std::list<PlayerBullet*>& GetBullet() { return playerBullet_; }
public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////

	//void SetWorldPos(Vector3 pos) { worldTransform_.translation_ = pos; }

	//void SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }

	const int32_t& GetHP() { return hp_; }
	const int32_t& GetMaxHP() { return maxHP_; }

private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////
	
	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	// ジャンプ処理
	void Jump();



private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	AudioManager::SoundData DamageSE;


	//移動限界座標
	static inline const Vector2 kMoveLimit_ = { 15.0f,15.0f };

	//弾
	std::list<PlayerBullet*> playerBullet_;

	//速度
	Vector3 velocity_ = {};
	//加速度
	Vector3 acceleration_ = {};
	//生存フラグ
	bool isAlive_ = true;
	//攻撃間隔
	int32_t attackInterval_ = 0;


	// ジャンプ用の変数
	bool isJumping_ = false;       // ジャンプ中かどうか
	float jumpVelocity_ = 0.0f;    // ジャンプの初速度
	const float jumpForce_ = 0.5f; // ジャンプ力
	const float gravity_ = -0.02f; // 重力
	float playerfloor_;

	// 体力
	int32_t hp_;
	const int32_t maxHP_ = 10;
	bool isHit = false;
	// 被弾クールタイム
	int32_t damageCoolTime_;
	Vector4 color_;

	//パーティクル発生器
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;

	int32_t shakingTime_ = 60;
};