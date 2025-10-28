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
#include "application/Entity/GameCharacterInfo.h"
#include "application/Provider/EnemyInputProvider.h"
#include "application/Entity/Behavior/BehaviorManager.h"
#include "application/Entity/Enemy/AIBrainSystem.h"
#include "application/Entity/Enemy/BulletSensor.h"
#include "application/Effect/DeadEffect.h"

//============================================================================
// Enemy class
//============================================================================
class Enemy : public GameCharacter {

public:
	Enemy() = default;
	~Enemy() override = default;

	//================================================================================
	// functions
	//================================================================================

	//初期化
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	//更新
	void Update() override;
	//ImGuiの更新
	void UpdateImGui();
	//描画
	void Draw() override;
	//コライダーの描画
	void DrawCollider() override;
	//衝突時の処理
	void OnCollisionAction(GameCharacter* other) override;

	//武器の初期化
	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager);

	/// <summary>
	/// EnemyのGameCharacterContextの読み込み
	/// </summary>
	/// <param name="characterName"></param>
	void LoadEnemyData(const std::string& characterName);

	/// <summary>
	/// EnemyのGameCharacterContextの保存
	/// </summary>
	/// <param name="characterName"></param>
	void SaveEnemyData(const std::string& characterName);
public:

	//==============================================================================
	// getter
	//==============================================================================

	//武器の取得
	BaseWeapon* GetWeapon(int index) const;
	//全武器の取得
	std::vector<std::unique_ptr<BaseWeapon>>& GetWeapons();
	//BulletSensorの取得
	BulletSensor* GetBulletSensor() const { return bulletSensor_.get(); }

	//移動方向ベクトルの取得
	const Vector3& GetMoveDirection() const { return characterInfo_.moveDirection; }
	//移動ベクトルの取得
	const Vector3& GetVelocity() const { return characterInfo_.velocity; }
	//transformの取得
	const QuaternionTransform& GetTransform() const { return characterInfo_.transform; }

	//体力の取得
	float GetHealth() const { return characterInfo_.health; }
	//最大体力の取得
	const float GetMaxHealth() const { return characterInfo_.maxHealth; }
	//フォーカス対象の座標を取得
	const Vector3& GetFocusTargetPos() const { return characterInfo_.focusTargetPos; }

	//エネルギーの取得
	float GetEnergy() const { return characterInfo_.energyInfo.energy; }
	//最大エネルギーの取得
	float GetMaxEnergy() const { return characterInfo_.energyInfo.maxEnergy; }
	//エネルギーの回復速度の取得
	float GetEnergyRegenRate() const { return characterInfo_.energyInfo.recoveryRate; }
	//エネルギー枯渇中かどうか
	bool GetIsOverHeated() const { return characterInfo_.overHeatInfo.isOverheated; }
	//生存しているかどうか
	bool GetIsAlive() const { return characterInfo_.isAlive; }

	//周回角度の取得
	float GetOrbitAngle() const { return orbitAngle_; }
	//周回半径の取得
	float GetOrbitRadius() const { return orbitRadius_; }
	//周回速度の取得
	float GetOrbitSpeed() const { return orbitSpeed_; }

	//================================================================================
	// setter
	//================================================================================

	//フォーカス対象の座標を設定
	void SetFocusTargetPos(const Vector3& targetPos) { characterInfo_.focusTargetPos = targetPos; }
	//移動ベクトルの設定
	void SetOrbitAngle(float angle) { orbitAngle_ = angle; }

	void SetFocusTargetVelocity(const Vector3& targetVel) { focusTargetVelocity_ = targetVel; }

private:

	//攻撃処理の更新
	void UpdateAttack();
	//各武器の攻撃処理
	void WeaponAttack(int weaponIndex);

	//エネルギーの更新
	void UpdateEnergy();

private:

	// チャージ攻撃実行判定
	bool ShouldReleaseAttack(int weaponIndex);

private:

	//AIシステム
	std::unique_ptr<AIBrainSystem> aiBrainSystem_ = nullptr;
	//接近してくる弾に反応するためのコライダー
	std::unique_ptr<BulletSensor> bulletSensor_ = nullptr;

	//状態管理マネージャ
	std::unique_ptr<BehaviorManager> behaviorManager_ = nullptr;
	//プレイヤー入力プロバイダ
	std::unique_ptr<EnemyInputProvider> inputProvider_ = nullptr;

	//プレイヤーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;
	//チャージ撃ちをする武器ユニット
	std::vector<bool> chargeShootableUnits_;

	//背部のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> backEmitter_ = nullptr;
	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;
	//死亡エフェクト
	std::unique_ptr<DeadEffect> deadEffect_ = nullptr;

	// プレイヤーの情報
	GameCharacterContext characterInfo_;
	// デルタタイム
	float deltaTime_ = 0.0f; 
	float gravity_ = 9.8f; // 重力の強さ
	
	float chargeShootDuration_ = 1.0f; // 停止撃ちの持続時間
	float chargeShootTimer_ = 0.0f; //停止撃ちまでの残り猶予時間
	float damageEffectTime_ = 0.0f; // ダメージエフェクトの時間

	//ターゲットの周りを周回するための変数
	float orbitAngle_ = 0.0f;              // 周回角度
	float orbitRadius_ = 60.0f;             // 周回半径（ターゲットとの距離）
	float orbitSpeed_ = 1.0f;              // 角速度（周る速さ）
	Vector3 toOrbitPos_ = { 0.0f,0.0f,0.0f }; // 周回する座標

	Vector3 focusTargetVelocity_ = { 0.0f,0.0f,0.0f }; // フォーカス対象の移動ベクトル
};