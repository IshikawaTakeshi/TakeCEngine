#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Entity/GameCharacter.h"
#include "Weapon/WeaponType.h"
#include "Weapon/WeaponData.h"
#include "3d/Particle/ParticleEmitter.h"
#include "engine/audio/Audio.h"
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
	virtual void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) = 0;
	//武器の更新
	virtual void Update() = 0;
	//ImGuiの更新
	virtual void UpdateImGui() = 0;
	//武器の描画
	virtual void Draw() = 0;
	void DrawShadow(const LightCameraInfo& lightCamera);
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
	virtual const WeaponType& GetWeaponType() const;
	//攻撃間隔の取得
	virtual float GetAttackInterval() const;

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
	virtual bool GetIsAvailable() const;
	//リロード中かどうか
	virtual bool GetIsReloading() const;

	//チャージ攻撃可能か
	virtual bool CanChargeAttack() const;
	//移動撃ち可能か
	virtual bool CanMoveShootable() const;
	//停止撃ち専用か
	virtual bool StopShootOnly() const;

	//=============================================================================
	// setter
	//=============================================================================

	//所有者の設定
	void SetOwnerObject(GameCharacter* owner) { ownerObject_ = owner; }
	//攻撃対象の座標を設定
	void SetTarget(const Vector3& targetPos) { targetPos_ = targetPos; }
	//攻撃対象の速度ベクトルを設定
	void SetTargetVelocity(const Vector3& targetVel) { targetVelocity_ = targetVel; }

	void SetRotate(const Vector3& rotate) { object3d_->SetRotate(rotate); }
	//武器のユニットポジションを設定
	void SetUnitPosition(uint32_t position);
	//有効射程距離の設定
	void SetEffectiveRange(float range);

	//弾速の設定
	void SetBulletSpeed(float speed);
	//攻撃力の設定
	void SetAttackPower(float power);
	//弾数の設定
	void SetBulletCount(int32_t count);
	//残弾数の設定
	void SetRemainingBulletCount(int32_t count);


protected:

	// 弾管理クラス
	BulletManager* bulletManager_ = nullptr;

	//武器の3Dオブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr;

	std::unique_ptr<ParticleEmitter> muzzleFlashEmitter_ = nullptr;

	//武器の親Joint名
	std::string parentJointName_;
	// 親スケルトン
	Skeleton* parentSkeleton_ = nullptr; 

	//武器の所有者オブジェクト
	GameCharacter* ownerObject_ = nullptr;
	//攻撃対象の座標
	Vector3 targetPos_;
	//攻撃対象の速度ベクトル
	Vector3 targetVelocity_;

	//武器の種類
	WeaponType weaponType_ = WeaponType::WEAPON_TYPE_RIFLE;
	//武器の基本データ
	WeaponData weaponData_{};
	WeaponState weaponState_{};
	// 武器のユニットポジション
	uint32_t unitPosition_ = 0; 

	//サウンドデータ
	AudioManager::SoundData shotSE_;
	float seVolume_ = 0.05f;
};