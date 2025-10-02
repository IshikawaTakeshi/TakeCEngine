#include "BoostEffect.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Easing.h"

void BoostEffect::Initialize(GameCharacter* owner) {
	boostEffectObject_ = std::make_unique<Object3d>();
	boostEffectObject_->Initialize(Object3dCommon::GetInstance(), "boostEffectCone.gltf");
	ownerObject_ = owner;
}

void BoostEffect::Update() {

	if(isActive_) {
	
		effectTime_ += TakeCFrameWork::GetDeltaTime();
		if(effectTime_ >= duration_) {
			effectTime_ = 0.0f;
			isActive_ = false;
		}
		float progress = effectTime_ / duration_;
		float easedProgress = Easing::EaseOutCubic(progress);
		alpha_ = 1.0f - easedProgress; // アルファ値を進行度に基づいて設定
		Vector3 newScale = activeScale_ * (0.5f + 0.5f * (1.0f - easedProgress)); // スケールを進行度に基づいて設定
		boostEffectObject_->SetScale(newScale);
		boostEffectObject_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
	}
	else {
		effectTime_ = 0.0f;
		alpha_ = 0.0f;
		boostEffectObject_->SetScale({ 0.0f,0.0f,0.0f });
		boostEffectObject_->GetModel()->GetMesh()->GetMaterial()->SetAlpha(alpha_);
	}

	if(parentSkeleton_ && !parentJointName_.empty() && ownerObject_) {
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		boostEffectObject_->SetParent(*jointWorldMatrixOpt);
	}

	boostEffectObject_->Update();
}

void BoostEffect::UpdateImGui(std::string label) {
	if(ImGui::TreeNode(label.c_str())) {
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

void BoostEffect::Draw() {
	if (isActive_) {
		boostEffectObject_->Draw();
	}
}

void BoostEffect::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;
}