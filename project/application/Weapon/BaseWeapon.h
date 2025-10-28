#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Entity/GameCharacter.h"
#include "Weapon/WeaponType.h"
#include <cstdint>
#include <string>
#include <memory>

//前方宣言
class BulletManager;

//============================================================================
// BaseWeapon class
//============================================================================
class BaseWeapon {
public:

	BaseWeapon() = default;
	virtual ~BaseWeapon() = default;

	//武器の初期化
	virtual void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) = 0;
	//武器の更新
	virtual void Update() = 0;
	//ImGuiの更新
	virtual void UpdateImGui() = 0;
	//武器の描画
	virtual void Draw() = 0;
	//武器の攻撃
	virtual void Attack() = 0;
	//武器のチャージ処理
	virtual void Charge([[maybe_unused]] float deltaTime) {};
	virtual void ChargeAttack() {};

	virtual void AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName);

public:

	//===========================================================================
	// getter
	//===========================================================================

	//武器タイプの取得
	virtual const WeaponType& GetWeaponType() const { return weaponType_; }
	//攻撃間隔の取得
	virtual float GetAttackInterval() const { return attackInterval_; }

	virtual const Vector3& GetTranslate() const { return object3d_->GetTranslate(); }

	virtual const Vector3& GetCenterPosition() const { return object3d_->GetCenterPosition(); }
	//ターゲットの座標を取得
	virtual const Vector3& GetTargetPos() const;
	//武器のユニットポジションを取得
	virtual uint32_t GetUnitPosition() const;
	//有効射程距離の取得
	virtual float GetEffectiveRange() const;

	//弾速の取得
	virtual float GetBulletSpeed() const;
	//攻撃力の取得
	virtual float GetAttackPower() const;
	//弾数の取得
	virtual uint32_t GetBulletCount() const;
	//最大弾数の取得
	virtual uint32_t GetMaxBulletCount() const;
	//一度に撃てる弾容量の取得
	virtual uint32_t GetMagazineCount() const;
	//残弾数の取得
	virtual uint32_t GetRemainingBulletCount() const;
	// チャージ中かどうか
	virtual bool IsCharging() const;
	// チャージ時間を取得
	virtual float GetChargeTime() const;
	// 必要チャージ時間を取得
	virtual float GetRequiredChargeTime() const;
	//使用可能かどうか
	virtual bool GetIsAvailable() const { return isAvailable_; }
	//リロード中かどうか
	virtual bool GetIsReloading() const { return isReloading_; }

	//チャージ攻撃可能か
	virtual bool IsChargeAttack() const = 0;
	//移動撃ち可能か
	virtual bool IsMoveShootable() const = 0;
	//停止撃ち専用か
	virtual bool IsStopShootOnly() const = 0;

	//=============================================================================
	// setter
	//=============================================================================

	//所有者の設定
	virtual void SetOwnerObject(GameCharacter* owner) { ownerObject_ = owner; }
	//攻撃対象の座標を設定
	virtual void SetTarget(const Vector3& targetPos) { targetPos_ = targetPos; }
	//攻撃対象の速度ベクトルを設定
	virtual void SetTargetVelocity(const Vector3& targetVel) { targetVel_ = targetVel; }

	virtual void SetRotate(const Vector3& rotate) { object3d_->SetRotate(rotate); }
	//武器のユニットポジションを設定
	virtual void SetUnitPosition(uint32_t position);
	//有効射程距離の設定
	virtual void SetEffectiveRange(float range);

	//弾速の設定
	virtual void SetBulletSpeed(float speed);
	//攻撃力の設定
	virtual void SetAttackPower(float power);
	//弾数の設定
	virtual void SetBulletCount(int32_t count);
	//残弾数の設定
	virtual void SetRemainingBulletCount(int32_t count);


protected:

	// 弾管理クラス
	BulletManager* bulletManager_ = nullptr;

	//武器の3Dオブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr;

	//武器の親Joint名
	std::string parentJointName_;
	// 親スケルトン
	Skeleton* parentSkeleton_ = nullptr; 

	//武器の所有者オブジェクト
	GameCharacter* ownerObject_ = nullptr;
	//攻撃対象の座標
	Vector3 targetPos_;
	//攻撃対象の速度ベクトル
	Vector3 targetVel_;

	//武器の種類
	WeaponType weaponType_ = WeaponType::WEAPON_TYPE_RIFLE;
	//武器の攻撃力
	float damage_ = 0.0f;
	//攻撃間隔
	float attackInterval_ = 0.0f;
	//弾のスピード
	float bulletSpeed_ = 0.0f;
	//使用可能か
	bool isAvailable_ = true;
	// 武器のユニットポジション
	uint32_t unitPosition_ = 0; 
	// 有効射程距離
	float effectiveRange_ = 0.0f;

	//リロード中かどうか
	bool isReloading_ = false;
	//リロード時間
	float reloadTime_ = 0.0f; // リロード時間
	float maxReloadTime_ = 0.0f; // 最大リロード時間

	uint32_t bulletCount_ = 0; // 現在の弾数
	uint32_t magazineCount_ = 0; // マガジン内の弾数
	uint32_t remainingBulletCount_ = 0; // 残弾数
	uint32_t maxBulletCount_ = 0; // 最大弾数

	// チャージ攻撃
	bool isCharging_ = false;  // チャージ攻撃フラグ
	float chargeTime_ = 0.0f;    // チャージ時間
	float requiredChargeTime_ = 0.0f; // 必要チャージ時間

	bool canChargeAttack_  = false; // チャージ攻撃フラグ
	bool canMoveShootable_ = false; // 移動撃ち可能か
	bool isStopShootOnly_ = false; // 停止撃ち専用か
};