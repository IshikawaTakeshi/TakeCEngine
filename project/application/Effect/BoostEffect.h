#pragma once
#include "engine/3d/Object3d.h"
#include <string>

class GameCharacter;

//============================================================================
// BoostEffect class
//============================================================================
class BoostEffect {
public:

	BoostEffect() = default;
	~BoostEffect() = default;

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

	//エフェクトのアクティブ設定
	void SetOwnerObject(GameCharacter* owner) { ownerObject_ = owner; }
	void SetIsActive(bool isActive) { isActive_ = isActive; }

private:

	//ブーストエフェクトオブジェクト
	std::unique_ptr<Object3d> boostEffectObject_ = nullptr;
	//武器の親Joint名
	std::string parentJointName_;
	// 親スケルトン
	Skeleton* parentSkeleton_ = nullptr; 
	//エフェクトの所有者
	GameCharacter* ownerObject_ = nullptr;

	//アクティブ時の可変スケール
	Vector3 activeScale_ = { 1.0f,1.0f,1.0f };
	//アルファ
	float alpha_ = 0.0f;
	// エフェクトの持続時間
	float duration_ = 0.5f;
	float effectTime_ = 0.0f;
	//エフェクトのアクティブフラグ
	bool isActive_ = false;
};

