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
void BoostEffect::Initialize(GameCharacter* owner) {
	ownerObject_ = owner;

	//EffectGroup の初期化
	effectGroup_ = std::make_unique<EffectGroup>();
	effectGroup_->Initialize("BoostEffect_Player.json");

	// --- 既存のMesh初期化 (そのまま残す場合) ---
	boostEffectObject_ = std::make_unique<Object3d>();
	boostEffectObject_->Initialize(&Object3dCommon::GetInstance(), "boostEffectCone.gltf");
	boostEffectObject_->GetModel()->GetModelData()->material.textureFilePath = "BlueBoostEffect.png";

	boostEffectObject2_ = std::make_unique<Object3d>();
	boostEffectObject2_->Initialize(&Object3dCommon::GetInstance(), "boostEffectCone.gltf");
	boostEffectObject2_->GetModel()->GetModelData()->material.textureFilePath = "BlueBoostEffect.png";
	boostEffectObject2_->SetScale({ 1.1f,1.1f,1.1f });

	// --- PointLight設定 ---
	pointLightData_.enabled_ = 1;
	pointLightData_.color_ = { 0.1f,0.5f,1.0f,1.0f };
	pointLightData_.intensity_ = 160.0f; // 最初は消しておく
	pointLightData_.radius_ = 2.0f;
	pointLightData_.decay_ = 1.0f;
	pointLightIndex_ = TakeCFrameWork::GetLightManager()->AddPointLight(pointLightData_);
}

//===================================================================================
//　更新
//===================================================================================
void BoostEffect::Update() {

	// 1. 親行列の取得と設定
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

	// 2. EffectGroupへの行列登録
	if (hasParent) {
		currentJointMatrix_ = jointWorldMatrix; // メンバ変数に保存
		effectGroup_->SetParentMatrix(&currentJointMatrix_); // アドレスを渡す

		// Meshも追従
		boostEffectObject_->SetParent(currentJointMatrix_);
		boostEffectObject2_->SetParent(currentJointMatrix_);
	} else {
		effectGroup_->SetParentMatrix(nullptr);
	}

	// 3. 挙動制御 (既存コード)
	if (isActive_) {
		effectTime_ += TakeCFrameWork::GetDeltaTime();
		// ... (既存の処理)
	}

	// 4. 更新実行
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);
	effectGroup_->Update();

	if (isActive_) {
		boostEffectObject_->Update();
		boostEffectObject2_->Update();
	}
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
		effectGroup_->Play(boostEffectObject_->GetCenterPosition());
		effectTime_ = 0.0f;
	} else {
		// 停止
		effectGroup_->Stop();
	}
}

//===================================================================================
//　描画
//===================================================================================
void BoostEffect::Draw() {
	if (isActive_) {
		// Mesh描画
		boostEffectObject_->Draw();
	}
}

//===================================================================================
//　アタッチ
//===================================================================================
void BoostEffect::AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName) {
	parentSkeleton_ = skeleton;
	parentJointName_ = jointName;

}