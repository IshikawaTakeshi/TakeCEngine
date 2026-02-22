#pragma once
#include "engine/3d/Object3d.h"
#include "engine/3d/Light/PointLight.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "engine/3d/Particle/EffectGroup.h" 
#include <string>
#include <memory>

class GameCharacter;

//===================================================================================
// BoostEffect class 
//===================================================================================
class BoostEffect {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BoostEffect() = default;
	~BoostEffect();

	//===================================================================================
	// functions
	//===================================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="owner"></param>
	void Initialize(GameCharacter* owner);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void UpdateImGui([[maybe_unused]] const std::string& windowName = nullptr);

	/// <summary>
	/// スケルトンのジョイントにエフェクトをアタッチ
	/// </summary>
	/// <param name="skeleton"></param>
	/// <param name="jointName"></param>
	void AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName);

	//===================================================================================
	// accessors
	//===================================================================================

	//----- setter ---------------------------
	void SetOwnerObject(GameCharacter* owner) { ownerObject_ = owner; }
	void SetIsActive(bool isActive); 
	void SetBehavior(GameCharacterBehavior behavior) { behavior_ = behavior; }

private:

	//EffectGroup のインスタンス
	std::unique_ptr<TakeC::EffectGroup> effectGroup_ = nullptr;

	// 親関連
	std::string parentJointName_;
	Skeleton* parentSkeleton_ = nullptr; 
	GameCharacter* ownerObject_ = nullptr;
	// ジョイントのワールド行列
	Matrix4x4 currentJointMatrix_;

	GameCharacterBehavior behavior_ = GameCharacterBehavior::NONE;

	// ライト
	PointLightData pointLightData_;
	uint32_t pointLightIndex_ = 0;

	// 制御用変数
	bool isActive_ = false;
	float effectTime_ = 0.0f;
	float duration_ = 0.5f;
	Vector3 activeScale_ = { 1.0f,1.0f,1.0f };
};