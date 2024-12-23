#pragma once
#include "Collision/Collider.h"

#include "Enemy/EnemyBullet.h"
#include "Sprite.h"
#include "Camera.h"
#include <list>
class Enemy : public Collider {
public:

	Enemy() = default;
	~Enemy();

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

public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		getter
	////////////////////////////////////////////////////////////////////////////////////////


	/// <summary>
	/// velocityの取得
	/// </summary>
	const Vector3& GetVelocity() { return velocity_; }

	/// <summary>
	/// ワールド座標の取得
	/// </summary>
	Vector3 GetWorldPos() override;


	const float GetRadius() { return radius_; }

	/// <summary>
	/// 生存フラグの取得
	/// </summary>
	bool GetIsAlive() { return isAlive_; }

	/// <summary>
	///	弾リストの取得
	/// </summary>
	const std::list<EnemyBullet*>& GetBullet() { return enemyBullet_; }
public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////


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

private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	//移動限界座標
	static inline const Vector2 kMoveLimit_ = { 15.0f,15.0f };

	//弾
	std::list<EnemyBullet*> enemyBullet_;

	//速度
	Vector3 velocity_ = {};
	//加速度
	Vector3 acceleration_ = {};
	//生存フラグ
	bool isAlive_ = true;
	//攻撃間隔
	int32_t attackInterval_ = 0;
	int32_t attackTime_ = 0;

	// 弾発射間隔
	int32_t bulletIntervalTimer_ = 0;

	// 体力
	int32_t hp_;
	const int32_t maxHP_ = 30;

	Vector4 color_;
	// 被弾フラグ
	bool isHit = false;
	// 被弾クールタイム
	int32_t damageCoolTime_;
};