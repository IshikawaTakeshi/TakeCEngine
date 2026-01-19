#include "MuzzleFlashEffect.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/3d/Object3dCommon.h"
#include "application/Weapon/BaseWeapon.h"

void MuzzleFlashEffect::Initialize() {
	//エフェクトオブジェクト初期化
	muzzleFlushEffectObjectIndices_.clear();
	for (int i = 0; i < 3; ++i) {
		uint32_t index = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(3.0f, 3.0f, "MazzleEffect1.png");
		muzzleFlushEffectObjectIndices_.push_back(index);
	}

	//エミッター初期化
	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("MuzzleFlushEffectEmitter", "RifleMuzzleFlash.json");
	particleEmitter_->SetParticleName("RifleMuzzleFlash");
	particleEmitter_->SetFrequency(0.01f);
	particleEmitter_->SetParticleCount(20);

	//pointLight設定
	pointLightData_.enabled_ = 1;
	pointLightData_.color_ = { 1.0f,0.5f,0.0f,1.0f };
	pointLightData_.intensity_ = 10.0f;
	pointLightData_.radius_ = 2.0f;
	pointLightData_.decay_ = 1.0f;
	//ポイントライトの追加
	pointLightIndex_ = TakeCFrameWork::GetLightManager()->AddPointLight(pointLightData_);

}

void MuzzleFlashEffect::Update() {

	effectTimer_.Update();

	if (!isActive_) {
		pointLightData_.enabled_ = 0;
		return;
	}
	
	if(effectTimer_.IsFinished()) {
		isActive_ = false;
		effectTimer_.Reset();
		pointLightData_.enabled_ = 0;
		return;
	}
	//アルファ計算
	alpha_ = 1.0f - (effectTimer_.GetProgress());
	pointLightData_.intensity_ = 10.0f * alpha_;

	//パーティクルエミッター更新
	particleEmitter_->SetIsEmit(isActive_);
	particleEmitter_->SetTranslate(ownerWeapon_->GetCenterPosition());
	//particleEmitter_->Update();
	//TakeCFrameWork::GetParticleManager()->GetParticleGroup("RifleMuzzleFlash")->SetEmitterPosition(ownerWeapon_->GetCenterPosition());
	//ポイントライト更新
	pointLightData_.position_ = ownerWeapon_->GetCenterPosition();
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);
}

void MuzzleFlashEffect::Emit() {
	isActive_ = true;
	effectTimer_.Initialize(0.048f, 0.0f);
}

void MuzzleFlashEffect::Draw() {
	if (!isActive_) {
		return;
	}
	//エフェクトオブジェクト描画
	for (const auto& index : muzzleFlushEffectObjectIndices_) {
		TakeCFrameWork::GetPrimitiveDrawer()->DrawObject(Object3dCommon::GetInstance().GetPSO(), PRIMITIVE_PLANE, index);
	}
	
}