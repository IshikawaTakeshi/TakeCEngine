#include "BoostEffect.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Easing.h"
#include <algorithm>

//===================================================================================
//　初期化
//===================================================================================
void BoostEffect::Initialize(GameCharacter* owner) {
	//effectObject初期化
	boostEffectObject_ = std::make_unique<Object3d>();
	boostEffectObject_->Initialize(Object3dCommon::GetInstance(), "boostEffectCone.gltf");
	boostEffectObject_->GetModel()->GetModelData()->material.textureFilePath = "BlueBoostEffect.png";
	//effectObject2初期化
	boostEffectObject2_ = std::make_unique<Object3d>();
	boostEffectObject2_->Initialize(Object3dCommon::GetInstance(), "boostEffectCone.gltf");
	boostEffectObject2_->GetModel()->GetModelData()->material.textureFilePath = "BlueBoostEffect.png";
	boostEffectObject2_->SetScale({ 1.1f,1.1f,1.1f });

	//エミッター初期化
	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("BoostEffectEmitter", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, {0.0f,0.0f,0.0f} }, 10, 0.1f);
	particleEmitter_->SetParticleName("BoostEffect3");
	ownerObject_ = owner;
}

//===================================================================================
//　更新
//===================================================================================
void BoostEffect::Update() {

	switch (behavior_) {
	case GameCharacterBehavior::NONE:
		break;
	case GameCharacterBehavior::IDLE:
	{

		//スケールを徐々に小さくしていく
		if (isActive_) {
			effectTime_ += TakeCFrameWork::GetDeltaTime();
			if (effectTime_ >= duration_) {
				effectTime_ = 0.0f;
				isActive_ = false;
			}

		} else {
			//スケール拡大
			effectTime_ = 0.0f;
			boostEffectObject_->SetScale({ 0.0f,0.0f,0.0f });
			boostEffectObject_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
			boostEffectObject2_->SetScale({ 0.0f,0.0f,0.0f });
			boostEffectObject2_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
		}
	}
		break;
	case GameCharacterBehavior::RUNNING:
	{
		//一定のスケールを保つ
		if (isActive_) {
			effectTime_ += TakeCFrameWork::GetDeltaTime();
			if (effectTime_ >= duration_) {
				effectTime_ = 0.0f;
				isActive_ = false;
			}
		}

		// アルファ値を進行度に基づいて設定
		float progress = effectTime_ / duration_;
		alpha_ = 1.0f; 
		float wave = sinf(progress * std::numbers::pi_v<float> *3.0f) * 0.1f;
		// 最終的なスケール係数
		float finalScaleFactor = 1.0f + wave; 
		// スケールを進行度に基づいて設定
		Vector3 newScale = activeScale_ * finalScaleFactor; 

		boostEffectObject_->SetScale(newScale);
		boostEffectObject_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
		boostEffectObject2_->SetScale(newScale * 1.1f);
		boostEffectObject2_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
		
	}
		break;
	case GameCharacterBehavior::JUMP:
	{


	}
		break;
	case GameCharacterBehavior::DASH:

	{
	
	}
		break;
	case GameCharacterBehavior::HEAVYDAMAGE:
	{

	}
		break;
	case GameCharacterBehavior::STEPBOOST:
	{

		if (isActive_) {

			effectTime_ += TakeCFrameWork::GetDeltaTime();
			if (effectTime_ >= duration_) {
				effectTime_ = 0.0f;
				isActive_ = false;
			}
			
		} else {
			effectTime_ = 0.0f;
			alpha_ = 0.0f;
			boostEffectObject_->SetScale({ 0.0f,0.0f,0.0f });
			boostEffectObject_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
			boostEffectObject2_->SetScale({ 0.0f,0.0f,0.0f });
			boostEffectObject2_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
		}
	}
		break;
	case GameCharacterBehavior::FLOATING:
	{

	}
		break;
	default:
	{

	}

		break;
	}

	
	if (parentSkeleton_ && !parentJointName_.empty() && ownerObject_) {
		// 親スケルトンのジョイントに追従
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		boostEffectObject_->SetParent(*jointWorldMatrixOpt);
		boostEffectObject2_->SetParent(*jointWorldMatrixOpt);
	}

	//effectObject更新
	boostEffectObject_->Update();
	boostEffectObject2_->Update();
	
	//particleエミッター更新
	particleEmitter_->SetIsEmit(isActive_);
	particleEmitter_->SetTranslate(boostEffectObject_->GetCenterPosition());
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("BoostEffect2")->SetEmitterPosition(boostEffectObject_->GetCenterPosition());
	particleEmitter_->Update();
}

//===================================================================================
//　ImGuiの更新
//===================================================================================
void BoostEffect::UpdateImGui(std::string label) {
	if (ImGui::TreeNode(label.c_str())) {
		ImGui::Checkbox("IsActive", &isActive_);
		Vector3 scale = boostEffectObject_->GetScale();
		Vector3 rotate = boostEffectObject_->GetRotate();
		ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &rotate.x, 0.01f);
		boostEffectObject_->SetScale(scale);
		boostEffectObject_->SetRotate(rotate);
		ImGui::TreePop();
	}
}

//===================================================================================
//　描画
//===================================================================================
void BoostEffect::Draw() {
	if (isActive_) {
		boostEffectObject_->Draw();
	}
}

//===================================================================================
//　スケルトンのジョイントにアタッチ
//===================================================================================
void BoostEffect::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
}

//===================================================================================
//　回転設定
//===================================================================================
void BoostEffect::SetRotate(const Vector3& rotate) {
	boostEffectObject_->SetRotate(rotate);
	boostEffectObject2_->SetRotate(rotate);
}