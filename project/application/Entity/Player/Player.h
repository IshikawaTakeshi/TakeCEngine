#pragma once
#include "engine/Entity/GameCharacter.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/WeaponType.h"
#include "camera/Camera.h"
#include "Particle/ParticleEmitter.h"
#include "Weapon/Bullet/BulletManager.h"
#include "engine/io/Gamepad.h"
#include <optional>

class Player : public GameCharacter {
public:
	Player() = default;
	~Player() override;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager);

public:

	//==============================================================================
	// getter
	//==============================================================================

	//移動方向ベクトルの取得
	const Vector3& GetMoveDirection() const { return moveDirection_; }
	//移動ベクトルの取得
	const Vector3& GetVelocity() const { return velocity_; }
	//transformの取得
	const QuaternionTransform& GetTransform() const { return transform_; }

	//体力の取得
	float GetHealth() const { return health_; }
	//最大体力の取得
	const float GetMaxHealth() const { return maxHealth_; }
	//フォーカス対象の座標を取得
	const Vector3& GetFocusTargetPos() const { return focusTargetPos_; }

	//エネルギーの取得
	float GetEnergy() const { return energy_; }
	//最大エネルギーの取得
	float GetMaxEnergy() const { return maxEnergy_; }
	//エネルギーの回復速度の取得
	float GetEnergyRegenRate() const { return energyRegenRate_; }

	bool GetIsOverHeated() const { return isOverheated_; }

	bool GetIsAlive() const { return isAlive_; }

	//================================================================================
	// setter
	//================================================================================

	//プレイヤーの体力を設定
	void SetHealth(float health) { health_ = health; }
	//フォーカス対象の座標を設定
	void SetFocusTargetPos(const Vector3& targetPos) { focusTargetPos_ = targetPos; }

private:

	enum class Behavior {
		IDLE,             // 待機状態
		RUNNING,          // 移動状態
		JUMP,             // ジャンプ状態
		DASH,             // ダッシュ状態
		CHARGESHOOT,      //チャージ攻撃中
		CHARGESHOOT_STUN, // チャージショット後の硬直状態
		HEAVYDAMAGE,	  // 大ダメージによる硬直状態
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
	void UpdateFloating();
	void UpdateDead();

	// ステップブーストのBehavior切り替え処理
	void TriggerStepBoost();
	//武器一つ当たりの攻撃処理
	void WeaponAttack(int weaponIndex, GamepadButtonType buttonType);

	//エネルギーの更新
	void UpdateEnergy();

private:

	//カメラ
	Camera* camera_ = nullptr;
	//状態遷移リクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//プレイヤーの状態
	Behavior behavior_ = Behavior::IDLE;
	Behavior prevBehavior_ = Behavior::IDLE;
	//プレイヤーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;

	//背部のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> backEmitter_ = nullptr;

	//補足対象の座標
	Vector3 focusTargetPos_ = { 0.0f,100.0f,0.0f };

	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };
	//transform
	QuaternionTransform transform_;
	// 重力の強さ
	const float gravity_ = 9.8f;
	// フレーム時間
	float deltaTime_ = 0.0f; 
	//移動速度
	const float moveSpeed_ = 200.0f;
	//移動速度の最大値
	const float kMaxMoveSpeed_ = 120.0f;

	//QBInfo
	Vector3 stepBoostDirection_ = { 0.0f,0.0f,0.0f }; // ステップブーストの方向
	const float stepBoostSpeed_     = 230.0f;         // ステップブーストの速度
	const float stepBoostDuration_  = 0.2f;           // ステップブーストの持続時間
	float stepBoostTimer_           = 0.0f;           // ステップブーストのタイマー
	float useEnergyStepBoost_ = 100.0f;         // ステップブーストに必要なエネルギー
	//インターバル用
	const float stepBoostInterval_ = 0.2f; // ステップブーストのインターバル
	float stepBoostIntervalTimer_  = 0.0f;  // ステップブーストのインターバルタイマー

	//JumInfo
	const float jumpSpeed_ = 50.0f;        // ジャンプの速度
	float jumpTimer_ = 0.0f;               // ジャンプのタイマー
	const float maxJumpTime_ = 0.5f;       // ジャンプの最大時間
	const float jumpDeceleration_ = 40.0f; // ジャンプ中の減速率
	float useEnergyJump_ = 100.0f;   // ジャンプに必要なエネルギー
	//落下速度
	float fallSpeed_ = 40.0f;

	// チャージ攻撃後硬直用の変数
	float chargeAttackStunTimer_ = 0.0f;          //チャージ攻撃後の硬直時間
	float chargeAttackStunDuration_ = 0.5f; // チャージ攻撃後の硬直時間

	//playerの体力
	float health_ = 0.0f;              // 現在の体力
	const float maxHealth_ = 10000.0f; // 最大体力

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

	bool isAlive_ = true; // プレイヤーが生存しているかどうか
};