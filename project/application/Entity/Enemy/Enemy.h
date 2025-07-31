#pragma once
#include "engine/Entity/GameCharacter.h"
#include "engine/camera/Camera.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include <optional>
#include <chrono>
#include <random>

#include "application/Weapon/Bullet/BulletManager.h"
#include "application/Weapon/BaseWeapon.h"
#include "application/Entity/GameCharacterBehavior.h"

class Enemy : public GameCharacter {

public:
	Enemy() = default;
	~Enemy() override;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager);

public:

	//setter
	void SetFocusTargetPos(const Vector3& targetPos) { focusTargetPos_ = targetPos; }
	//getter
	const QuaternionTransform& GetTransform() const { return transform_; }
	const Vector3& GetVelocity() const { return velocity_; }
	const Vector3& GetMoveDirection() const { return moveDirection_; }
	const Vector3& GetToOrbitPos() const { return toOrbitPos_; }
	const float& GetHealth() const { return health_; }
	const float& GetMaxHealth() const { return maxHealth_; }
	const bool& IsDamaged() const { return isDamaged_; }
	const bool& IsAlive() const { return isAlive_; }

private:

	//状態遷移の列挙型
	enum class Behavior {
		IDLE,             // 待機状態
		RUNNING,          // 移動状態
		JUMP,             // ジャンプ状態
		DASH,             // ダッシュ状態
		CHARGESHOOT,      //チャージ攻撃中
		CHARGESHOOT_STUN, // チャージショット後の硬直状態
		HEAVYDAMAGE,      // 大ダメージによる硬直状態
		STEPBOOST,        // ステップブースト
		FLOATING,         // 浮遊状態
		DEAD,             // 死亡状態
	};

	void InitRunning();
	void InitJump();
	void InitDash();
	void InitChargeShoot();
	void InitChargeShootStun();
	void InitStepBoost();
	void InitFloating();
	void InitDead();

	void UpdateRunning();
	void UpdateAttack();
	void UpdateDamage();
	void UpdateJump();
	void UpdateDash();
	void UpdateChargeShoot();
	void UpdateChargeShootStun();
	void UpdateStepBoost();
	void UpdateFloating(std::mt19937 randomEngine);
	void UpdateDead();

	// ステップブーストのBehavior切り替え処理
	void TriggerStepBoost();

	//エネルギーの更新
	void UpdateEnergy();

private:
	// 攻撃開始判定
	bool ShouldStartAttack(int weaponIndex);
	// チャージ攻撃実行判定
	bool ShouldReleaseAttack(int weaponIndex);

private:

	//状態遷移リクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//エネミーの状態
	Behavior behavior_ = Behavior::IDLE;
	Behavior prevBehavior_ = Behavior::IDLE;
	//エネミーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;
	//particleEmitter
	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;

	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };

	//補足対象の座標
	Vector3 focusTargetPos_ = { 0.0f,0.0f,0.0f };
	//補足スピード
	float followSpeed_ = 0.3f;
	//拡縮・回転・平行移動を含む変形情報
	QuaternionTransform transform_ = { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,0.0f} };

	//ゲーム内の1フレームの経過時間
	float deltaTime_ = 0.0f;
	//体力
	float health_ = 0.0f; // 初期体力
	const float maxHealth_ = 10000.0f; // 最大体力

	const float moveSpeed_ = 200.0f;    // 移動速度
	const float kMaxMoveSpeed_ = 50.0f; // 最大移動速度

	//ターゲットの周りを周回するための変数
	float orbitAngle_ = 0.0f;              // 周回角度
	float orbitRadius_ = 60.0f;             // 周回半径（ターゲットとの距離）
	float orbitSpeed_ = 1.0f;              // 角速度（周る速さ）
	Vector3 toOrbitPos_ = { 0.0f,0.0f,0.0f }; // 周回する座標

	//QBInfo
	Vector3 stepBoostDirection_ = { 0.0f,0.0f,0.0f }; // ステップブーストの方向
	const float stepBoostSpeed_ = 230.0f;             // ステップブーストの速度
	const float stepBoostDuration_ = 0.2f;            // ステップブーストの持続時間
	float stepBoostTimer_ = 0.0f;                     // ステップブーストのタイマー
	float useEnergyStepBoost_ = 100.0f;               // ステップブーストに必要なエネルギー
	//インターバル用
	const float stepBoostInterval_ = 0.2f; // ステップブーストのインターバル
	float stepBoostIntervalTimer_ = 0.0f;  // ステップブーストのインターバルタイマー

	//JumInfo
	const float jumpSpeed_ = 50.0f;        // ジャンプの速度
	float jumpTimer_ = 0.0f;               // ジャンプのタイマー
	const float maxJumpTime_ = 0.5f;       // ジャンプの最大時間
	const float jumpDeceleration_ = 40.0f; // ジャンプ中の減速率
	const float gravity_ = 9.8f;           // 重力の強さ
	float useEnergyJump_ = 100.0f;         // ジャンプに必要なエネルギー
	//落下速度
	float fallSpeed_ = 40.0f; // 落下速度

	// チャージ攻撃後硬直用の変数
	float chargeAttackStunTimer_ = 0.0f;          //チャージ攻撃後の硬直時間
	const float chargeAttackStunDuration_ = 0.5f; // チャージ攻撃後の硬直時間

	//エネルギー(スタミナ)情報
	float energy_ = 0.0f;                 // 現在のエネルギー
	float maxEnergy_ = 1000.0f;           // 最大エネルギー
	float energyRegenRate_ = 200.0f; // エネルギーの回復速度
	//エネルギー使用のクールダウン
	const float energyCooldown_ = 1.0f; // エネルギー使用後のクールダウン時間

	//オーバーヒート情報
	bool isOverheated_ = false;        // オーバーヒート中かどうか
	float overheatTimer_ = 0.0f;          // オーバーヒートのタイマー
	const float overheatDuration_ = 3.0f; // オーバーヒートの持続時間

	//ダメージを受けた時のエフェクト適用時間
	float damageEffectTime_ = 0.0f;

	//敵が攻撃する確率
	const float attackProbability_ = 10.0f; // 10%の確率で攻撃
	//ジャンプする確率
	const float jumpProbability_ = 1.0f;

	//状態遷移タイマー
	float stateTransitionTimer_ = 0.0f; // 状態遷移のタイマー

	bool isDamaged_ = false; //ダメージを受けたかどうか
	bool isAlive_ = true; //敵が生きているかどうか
};