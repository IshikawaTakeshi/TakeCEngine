#pragma once
#include "engine/Entity/GameCharacter.h"
#include "engine/Input/Gamepad.h"
#include "engine/camera/Camera.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include <optional>
#include <memory>

#include "application/Weapon/BaseWeapon.h"
#include "application/Weapon/WeaponType.h"
#include "application/Weapon/Bullet/BulletManager.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "application/Entity/GameCharacterInfo.h"
#include "application/Provider/PlayerInputProvider.h"
#include "application/Entity/Behavior/BehaviorManager.h"
#include "application/Effect/BoostEffect.h"

//==================================================================================
// Player class
//==================================================================================
class Player : public GameCharacter {
public:
	

	//==============================================================================
	// functions
	//==============================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	Player() = default;
	~Player() override = default;

	//初期化
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath);
	//更新
	void Update() override;
	//ImGuiの更新
	void UpdateImGui();
	//描画
	void Draw() override;
	void DrawShadow(const LightCameraInfo& lightCamera);
	//コライダーの描画
	void DrawCollider() override;
	//衝突時の処理
	void OnCollisionAction(GameCharacter* other) override;

	//武器の初期化
	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager);

	/// <summary>
	/// PlayerのGameCharacterContextの読み込み
	/// </summary>
	/// <param name="characterName"></param>
	void LoadPlayerData(const std::string& characterName);

	/// <summary>
	/// PlayerのGameCharacterContextの保存
	/// </summary>
	/// <param name="characterName"></param>
	void SavePlayerData(const std::string& characterName);

public:

	//==============================================================================
	// getter
	//==============================================================================

	//武器の取得
	BaseWeapon* GetCurrentWeapon(int index) const;
	//全武器の取得
	std::vector<std::unique_ptr<BaseWeapon>>& GetWeapons();

	//カメラの取得
	Camera* GetCamera() const { return camera_; }

	//移動方向ベクトルの取得
	const Vector3& GetMoveDirection() const { return playerData_.characterInfo.moveDirection; }
	//移動ベクトルの取得
	const Vector3& GetVelocity() const { return playerData_.characterInfo.velocity; }
	//胴体位置の取得
	const Vector3& GetBodyPosition() const { return bodyPosition_; }
	//transformの取得
	const QuaternionTransform& GetTransform() const { return playerData_.characterInfo.transform; }

	//体力の取得
	float GetHealth() const { return playerData_.characterInfo.health; }
	//最大体力の取得
	const float GetMaxHealth() const { return playerData_.characterInfo.maxHealth; }
	//フォーカス対象の座標を取得
	const Vector3& GetFocusTargetPos() const { return playerData_.characterInfo.focusTargetPos; }

	//エネルギーの取得
	float GetEnergy() const { return playerData_.characterInfo.energyInfo.energy; }
	//最大エネルギーの取得
	float GetMaxEnergy() const { return playerData_.characterInfo.energyInfo.maxEnergy; }
	//エネルギーの回復速度の取得
	float GetEnergyRegenRate() const { return playerData_.characterInfo.energyInfo.recoveryRate; }
	//エネルギー枯渇フラグの取得
	bool GetIsOverHeated() const { return playerData_.characterInfo.overHeatInfo.isOverheated; }
	//生存フラグの取得
	bool GetIsAlive() const { return playerData_.characterInfo.isAlive; }
	//フォーカス中かどうか
	bool IsFocus() const { return isFocus_; }

	bool IsInCombat() const { return playerData_.characterInfo.isInCombat; }

	//================================================================================
	// setter
	//================================================================================

	//プレイヤーの体力を設定
	void SetHealth(float health) { playerData_.characterInfo.health = health; }
	//フォーカス対象の座標を設定
	void SetFocusTargetPos(const Vector3& targetPos) { playerData_.characterInfo.focusTargetPos = targetPos; }

	void SetFocusTargetVelocity(const Vector3& targetVel) { focusTargetVelocity_ = targetVel; }

	void SetInCombat(bool inCombat) { playerData_.characterInfo.isInCombat = inCombat; }

	void SetInputProvider(PlayerInputProvider* inputProvider) { inputProvider_ = inputProvider; }

private:

	//カメラ
	Camera* camera_ = nullptr;
	//状態管理マネージャ
	std::unique_ptr<BehaviorManager> behaviorManager_ = nullptr;
	//プレイヤー入力プロバイダ(借りる)
	PlayerInputProvider* inputProvider_ = nullptr;

	//プレイヤーの武器
	std::vector<std::unique_ptr<BaseWeapon>> weapons_;
	std::vector<WeaponType> weaponTypes_;
	//チャージ撃ちをする武器ユニット
	std::vector<bool> chargeShootableUnits_;

	//背部のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> backEmitter_ = nullptr;
	//ブーストエフェクト
	std::vector<std::unique_ptr<BoostEffect>> boostEffects_;
	// プレイヤーの基本情報
	CharacterData playerData_;

	// フレーム時間
	float deltaTime_ = 0.0f; 
	
	float chargeShootDuration_ = 1.0f; // 停止撃ちの持続時間
	Timer chargeShootTimer_; // 停止撃ちのタイマー

	bool isUseWeapon_ = false; //武器を使用しているかどうか
	float weaponUseTimer_ = 0.0f; //武器を使用している時間
	float weaponUseDuration_ = 1.0f; //武器を使用してからのクールダウン時間

	bool isFocus_ = true; //フォーカス中かどうか
	Vector3 focusTargetVelocity_ = { 0.0f,0.0f,0.0f }; // フォーカス対象の移動ベクトル
	Vector3 bodyPosition_ = { 0.0f,0.0f,0.0f }; // キャラクターの胴体位置

private:

	//武器の攻撃更新
	void UpdateAttack();

	//武器一つ当たりの攻撃処理
	void WeaponAttack(CharacterActionInput actionInput);

	//エネルギーの更新
	void UpdateEnergy();

	//ブーストエフェクトのアクティブ化判定
	void RequestActiveBoostEffect();
};