#include "BoostEffect.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Math/MathEnv.h"
#include "engine/Math/MatrixMath.h"

using namespace TakeC; // 名前空間省略

BoostEffect::~BoostEffect() {
	TakeCFrameWork::GetLightManager()->RemovePointLight(pointLightIndex_);
}

//===================================================================================
//　初期化
//===================================================================================
void BoostEffect::Initialize(
	GameCharacter* owner,
	const std::string& effectName, 
	const std::string& appearEffectName, 
	const std::string& stepBoostEffectName) {
	ownerObject_ = owner;

	//EffectGroup の初期化
	thrusterEffect_ = std::make_unique<EffectGroup>();
	thrusterEffect_->Initialize(effectName);

	//AppearEffectの初期化
	appearEffect_ = std::make_unique<EffectGroup>();
	appearEffect_->Initialize(appearEffectName);

	//StepBoostEffectの初期化
	stepBoostEffect_ = std::make_unique<EffectGroup>();
	stepBoostEffect_->Initialize(stepBoostEffectName);

	// --- PointLight設定 ---
	pointLightData_.enabled_ = 1;
	pointLightData_.color_ = { 0.1f,0.5f,1.0f,1.0f };
	pointLightData_.intensity_ = 0.0f; // 最初は消しておく
	pointLightData_.radius_ = 2.0f;
	pointLightData_.decay_ = 1.0f;
	pointLightIndex_ = TakeCFrameWork::GetLightManager()->AddPointLight(pointLightData_);
	
	// 確実に停止処理を実行するため、一度 true にしておく
	isActive_ = true;
	SetIsActive(false);
}

//===================================================================================
//　更新
//===================================================================================
void BoostEffect::Update() {

	// 親行列の取得と設定
	// Skeletonからジョイントのワールド行列を計算して取得する必要がある
	Matrix4x4 jointWorldMatrix = MatrixMath::MakeIdentity4x4();
	bool hasParent = false;

	if (parentSkeleton_ && !parentJointName_.empty() && ownerObject_) {
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();

		// GetJointWorldMatrixがoptional<Matrix4x4>を返すと仮定
		auto matrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		if (matrixOpt.has_value()) {
			jointWorldMatrix = matrixOpt.value();
			hasParent = true;
		}
	}

	// EffectGroupへの行列登録
	if (hasParent) {
		currentJointMatrix_ = jointWorldMatrix;
		thrusterEffect_->SetParentMatrix(&currentJointMatrix_);
		appearEffect_->SetParentMatrix(&currentJointMatrix_);
		stepBoostEffect_->SetParentMatrix(&currentJointMatrix_);

	} else {
		thrusterEffect_->SetParentMatrix(nullptr);
		appearEffect_->SetParentMatrix(nullptr);
		stepBoostEffect_->SetParentMatrix(nullptr);
	}

	// 4. 更新実行
	pointLightData_.position_ = {
		currentJointMatrix_.m[3][0], currentJointMatrix_.m[3][1], currentJointMatrix_.m[3][2]
	};
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);
	thrusterEffect_->Update();
	appearEffect_->Update();
	stepBoostEffect_->Update();
}

void BoostEffect::UpdateImGui([[maybe_unused]] const std::string& windowName) {

	thrusterEffect_->UpdateImGui(windowName);
	appearEffect_->UpdateImGui(windowName);
	stepBoostEffect_->UpdateImGui(windowName);
}

//===================================================================================
//　アクティブ設定
//===================================================================================
void BoostEffect::SetIsActive(bool isActive) {
	if (isActive_ == isActive) return;

	isActive_ = isActive;

	if (isActive_) {
		// 再生開始
		// Meshの位置や回転がセットされていれば、その位置で再生開始
		thrusterEffect_->Play(pointLightData_.position_);
		pointLightData_.intensity_ = 160.0f;

	} else {
		// 停止
		thrusterEffect_->Stop();
		pointLightData_.intensity_ = 0.0f;
	}
}

//===================================================================================
//　アタッチ
//===================================================================================
void BoostEffect::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;

}

void BoostEffect::PlayAppearEffect() {
	appearEffect_->Play(pointLightData_.position_);
}

void BoostEffect::PlayStepBoostEffect() {
	stepBoostEffect_->Play(pointLightData_.position_);
}
