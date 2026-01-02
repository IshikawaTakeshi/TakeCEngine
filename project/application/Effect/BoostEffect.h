#pragma once
#include "engine/3d/Object3d.h"
#include "engine/3d/Light/PointLight.h"
#include "application/Entity/GameCharacterBehavior.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include <string>

class GameCharacter;

//============================================================================
// BoostEffect class
//============================================================================
class BoostEffect {
public:

	BoostEffect() = default;
	~BoostEffect();

	//初期化
	void Initialize(GameCharacter* owner);
	//更新
	void Update();
	//ImGuiの更新
	void UpdateImGui(std::string label);
	//描画
	void Draw();
	//スケルトンのジョイントにアタッチ
	void AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName);

	//----- getter ---------------------------

	Object3d* GetEffectObject() { return boostEffectObject_.get(); }

	//----- setter ---------------------------

	//エフェクトの所有者設定
	void SetOwnerObject(GameCharacter* owner) { ownerObject_ = owner; }
	//エフェクトのアクティブ設定
	void SetIsActive(bool isActive) { isActive_ = isActive; }
	//GameCharacterの行動を参照
	void SetBehavior(GameCharacterBehavior behavior) { behavior_ = behavior; }
	//エフェクトのスケール設定
	void SetRotate(const Vector3& rotate);

private:

	//ブーストエフェクトオブジェクト
	std::unique_ptr<Object3d> boostEffectObject_ = nullptr;
	std::unique_ptr<Object3d> boostEffectObject2_ = nullptr;
	//パーティクルエミッター
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;
	//武器の親Joint名
	std::string parentJointName_;
	// 親スケルトン
	Skeleton* parentSkeleton_ = nullptr; 
	//エフェクトの所有者
	GameCharacter* ownerObject_ = nullptr;

	GameCharacterBehavior behavior_;

	//アクティブ時の可変スケール
	Vector3 activeScale_ = { 1.0f,1.0f,1.0f };
	//アルファ
	float alpha_ = 0.0f;
	// エフェクトの持続時間
	float duration_ = 0.5f;
	float effectTime_ = 0.0f;
	//エフェクトのアクティブフラグ
	bool isActive_ = false;

	PointLightData pointLightData_;
	uint32_t pointLightIndex_ = 0;
};