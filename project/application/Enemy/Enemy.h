#pragma once
#include "Audio.h"
#include "Camera.h"
#include "Collision/Collider.h"
#include "application/Player/Player.h"
#include "ParticleEmitter.h"

#include <list>

class Sprite;
class EnemyBullet;

enum AttackType {
	CHARGE,
	FALLINGROCK,
	THROWROCK,
};

enum Phase {
	FIRST,
	SECOND,
	FINAL,
};

/// <summary>
/// プレイヤー
/// </summary>
class Enemy : public Collider {
public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	Enemy() = default;
	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath, Player* player);

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
	/// ImGuiでのデバッグ処理
	/// </summary>
	void ImGuiDebug();

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

	// const float GetRadius() { return kRadius; }

	/// <summary>
	/// 生存フラグの取得
	/// </summary>
	bool GetIsAlive() { return isAlive_; }

	/// <summary>
	///	弾リストの取得
	/// </summary>
	const std::list<EnemyBullet*>& GetBullet() { return enemyBullet_; }

	const int32_t& GetHP() { return hp_; }
	const int32_t& GetMaxHP() { return maxHP_; }

public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		setter
	////////////////////////////////////////////////////////////////////////////////////////

private:
	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	/// <summary>
	/// 突進攻撃
	/// </summary>
	void ChargeAttack();

	/// <summary>
	/// 落石攻撃
	/// </summary>
	void FallingRockAttack();

	/// <summary>
	/// 投石攻撃
	/// </summary>
	void ThrowRockAttack();

	/// <summary>
	/// 攻撃パターンを切り替える関数
	/// </summary>
	void ChangeAttackPattern();

	/// <summary>
	/// 無敵時間の更新
	/// </summary>
	void UpdateInvincibleTime();

	/// <summary>
	/// フェーズ切り替え
	/// </summary>
	void UpdatePhase();

	/// <summary>
	/// フェーズを数字から文字へ
	/// </summary>
	const char* PhaseToString(Phase phase);

	/// <summary>
	/// 攻撃タイプを数字から文字へ
	/// </summary>
	const char* AttackTypeToString(AttackType attackType);

private:
	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ変数
	////////////////////////////////////////////////////////////////////////////////////////

	AudioManager::SoundData throwRockSE;
	AudioManager::SoundData damageSE;

	// プレイヤー
	Player* player_ = nullptr;

	// 弾
	std::list<EnemyBullet*> enemyBullet_;

	Vector4 color_;

	// 速度
	Vector3 velocity_ = {};
	// 加速度
	Vector3 acceleration_ = {};
	// 生存フラグ
	bool isAlive_ = true;
	// 攻撃フラグ
	bool isAttack_ = false;
	// 攻撃間隔
	int32_t attackInterval_ = 180;
	int32_t attackTime_ = 0;

	// 弾発射間隔
	int32_t bulletIntervalTimer_ = 0;

	// 初期攻撃パターン
	AttackType currentAttackType_ = AttackType::CHARGE;

	// フェーズ
	Phase phase_ = Phase::FIRST;

	// 体力
	int32_t hp_;
	const int32_t maxHP_ = 30;

	// 被弾フラグ
	bool isHit = false;
	// 被弾クールタイム
	int32_t damageCoolTime_;

	//パーティクル発生器
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;

	int32_t deathTimer_;
	int32_t flyTimer_;
};