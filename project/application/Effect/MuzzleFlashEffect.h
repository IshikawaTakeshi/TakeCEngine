#pragma once
#include "engine/3d/Object3d.h"
#include "engine/3d/Light/PointLight.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include "engine/Utility/Timer.h"
#include <string>

class BaseWeapon;

class MuzzleFlashEffect {
public:
	//========================================================================
	// functions
	//========================================================================
	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	MuzzleFlashEffect() = default;
	~MuzzleFlashEffect() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void Emit();

	//========================================================================
	// accessors
	//========================================================================

	//----- setter ---------------

	//スケルトンのジョイントにアタッチ
	void AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName);
	//エフェクトの所有武器設定
	void SetOwnerWeapon(BaseWeapon* owner) { ownerWeapon_ = owner; }

private:

	//ブーストエフェクトオブジェクト
	std::vector<uint32_t> muzzleFlushEffectObjectIndices_;
	//パーティクルエミッター
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;
	//武器の親Joint名
	std::string parentJointName_;
	// 親スケルトン(まだ使わない)
	Skeleton* parentSkeleton_ = nullptr; 
	//エフェクトの所有武器
	BaseWeapon* ownerWeapon_ = nullptr;

	//アクティブ時の可変スケール
	Vector3 activeScale_ = { 1.0f,1.0f,1.0f };
	//アルファ
	float alpha_ = 0.0f;
	// エフェクトタイマー
	Timer effectTimer_;
	//エフェクトのアクティブフラグ
	bool isActive_ = false;

	PointLightData pointLightData_;
	uint32_t pointLightIndex_ = 0;
};

