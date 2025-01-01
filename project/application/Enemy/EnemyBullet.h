#pragma once
#include "Model.h"

#include "Camera.h"
#include "Collision/Collider.h"
#include "ParticleEmitter.h"

/// <summary>
/// 自キャラの弾
/// </summary>
class EnemyBullet : public Collider {
public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	EnemyBullet() = default;
	~EnemyBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const Vector3& playerPosition, const Vector3& velocity);

	/// <summary>
	/// 落石攻撃の更新処理
	/// </summary>
	void FallingRockUpdate();

	/// <summary>
	/// 直進岩攻撃の更新
	/// </summary>
	void StraightRockUpdate();

	/// <summary>
	/// ImGuiでのデバッグ表示
	/// </summary>
	void ImGuiDebug();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 衝突を検出したら呼び出されるコールバック関数
	/// </summary>
	void OnCollision([[maybe_unused]] Collider* other) override;

public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		getter
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// ワールド座標の取得
	/// </summary>
	Vector3 GetWorldPos() override;

	Vector3 GetTranslate();

	/// <summary>
	/// 弾の半径の取得
	/// </summary>
	const float GetRadius() const { return kRadius_; }


	const int32_t GetDeathTimer() const;

	/// <summary>
	/// isDeadの取得
	/// </summary>
	bool IsDead() const { return isDead_; }

public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////

	void SetIsDead(bool isDead) { isDead_ = isDead; }

private:
	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	// 弾の寿命
	static inline const int32_t kLifeTime_ = 200;

	// 弾の半径
	static inline const float kRadius_ = 0.9f;

	// モデル
	// Model* model_ = nullptr;

	// テクスチャ
	uint32_t textureHandle_;

	// 速度
	Vector3 velocity_ = {0, 0, 0};

	// デスタイマー
	int32_t deathTimer_ = kLifeTime_;

	// 生存フラグ
	bool isDead_ = false;

	// 重力の設定
	float gravity_ = -0.02f; // 調整可能

	//パーティクル発生器
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;
};