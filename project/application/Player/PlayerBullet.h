#pragma once
#include "Model.h"

#include "Camera.h"
#include "Collision/Collider.h"
#include "ParticleEmitter.h"

/// <summary>
/// 自キャラの弾
/// </summary>
class PlayerBullet : public Collider {
public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	PlayerBullet() = default;
	~PlayerBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const Vector3& playerPosition, const Vector3& velocity);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ワールド行列のみの更新処理
	/// </summary>
	//void UpdateMatrix() { worldTransform_.UpdateMatrix(); }

	/// <summary>
	/// ImGuiの更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 衝突を検出したら呼び出されるコールバック関数
	/// </summary>
	void OnCollision([[maybe_unused]] Collider* other) override;

	/// <summary>
	/// isDeadの取得
	/// </summary>
	bool IsDead() const { return isDead_; }

public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		getter
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// ワールド座標の取得
	/// </summary>
	Vector3 GetWorldPos() override;

	/// <summary>
	/// 弾の半径の取得
	/// </summary>
	const float GetRadius() const { return kRadius_; }

public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////

	//void SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }



private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	//弾の寿命
	static inline const int32_t kLifeTime_ = 200;

	//弾の半径
	static inline const float kRadius_ = 0.9f;

	//パーティクル発生器
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;

	//速度
	Vector3 velocity_ = { 0,0,0 };

	//デスタイマー
	int32_t deathTimer_ = kLifeTime_;

	//生存フラグ
	bool isDead_ = false;


	// 重力の設定
	float gravity_ = -0.02f; // 調整可能

	// 新しく追加: 跳ねた回数をカウントする変数
	int bounceCount_ = 0;
	static const int kMaxBounces = 1; // 最大跳ね回数
	float bulletfloor_;
};

