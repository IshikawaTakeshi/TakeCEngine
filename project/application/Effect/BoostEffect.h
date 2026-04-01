#pragma once
#include "engine/3d/Object3d.h"
#include "engine/3d/Light/PointLight.h"
#include "application/Entity/State/GameCharacterState.h"
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
	void Initialize(GameCharacter* owner,const std::string& effectName,const std::string& appearEffectName);

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

	void PlayAppearEffect();

	//===================================================================================
	// accessors
	//===================================================================================

	//----- setter ---------------------------
	void SetOwnerObject(GameCharacter* owner) { ownerObject_ = owner; }
	void SetIsActive(bool isActive); 
	void SetCharacterState(GameCharacterState behavior) { behavior_ = behavior; }

private:

	//EffectGroup のインスタンス
	std::unique_ptr<TakeC::EffectGroup> effectGroup_ = nullptr;
	// ブースト開始時のエフェクト
	std::unique_ptr<TakeC::EffectGroup> appearEffect_ = nullptr;

	// 親関連
	std::string parentJointName_;
	Skeleton* parentSkeleton_ = nullptr; 
	GameCharacter* ownerObject_ = nullptr;
	// ジョイントのワールド行列
	Matrix4x4 currentJointMatrix_;

	GameCharacterState behavior_ = GameCharacterState::NONE;

	// ライト
	PointLightData pointLightData_;
	uint32_t pointLightIndex_ = 0;

	// 制御用変数
	bool isActive_ = false;
	float duration_ = 0.5f;
	Vector3 activeScale_ = { 1.0f,1.0f,1.0f };
};