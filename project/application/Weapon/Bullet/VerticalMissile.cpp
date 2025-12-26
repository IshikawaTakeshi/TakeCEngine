#include "VerticalMissile.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/MatrixMath.h"
#include "engine/math/Easing.h"
#include "engine/Collision/SphereCollider.h"

//====================================================================================
// 初期化処理
//====================================================================================

void VerticalMissile::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//オブジェクト初期化
	if (!object3d_) {
		object3d_ = std::make_unique<Object3d>();
	}
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	if (!collider_) {
		collider_ = std::make_unique<SphereCollider>();
		collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	}
	collider_->SetRadius(bulletRadius_); // 半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Missile)); // 種別IDを設定
	//emiiter設定
	//emitter0
	particleEmitter_.resize(2);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 30, 0.001f);
	particleEmitter_[0]->SetParticleName("MissileExplosion");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 10, 0.001f);
	particleEmitter_[1]->SetParticleName("MissileSmoke");

	deltaTime_ = TakeCFrameWork::GetDeltaTime();
	

	//transform初期化
	transform_.translate = { 0.0f, 200.0f, 0.0f };
	bulletRadius_ = 1.5f; // 弾の半径を設定

	pointLightData_.color_ = { 1.0f,0.1f,0.2f,1.0f };
	pointLightData_.intensity_ = 120.0f;
	pointLightData_.radius_ = 20.0f;
	pointLightData_.decay_ = 6.0f;
	//ポイントライトの追加
	pointLightIndex_ = TakeCFrameWork::GetLightManager()->AddPointLight(pointLightData_);
}

//====================================================================================
// 更新処理
//====================================================================================

void VerticalMissile::Update() {

	deltaTime_ = TakeCFrameWork::GetDeltaTime();
	if (isActive_ == false) {
		pointLightData_.enabled_ = 0;
		return;
	}


	//ライフタイムの減少
	lifeTime_ -= deltaTime_;
	//ポイントライトの更新
	pointLightData_.position_ = transform_.translate;
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);

	//ライフタイムが0以下になったら無効化
	if (lifeTime_ <= 0.0f) {
		pointLightData_.enabled_ = 0;
		isActive_ = false;
		return;
	}

	switch (phase_) {
	case VerticalMissile::VerticalMissilePhase::ASCENDING:
		// 上昇中の処理
		transform_.translate.y += vmInfo_.ascendSpeed * TakeCFrameWork::GetDeltaTime();
		if (transform_.translate.y >= vmInfo_.maxAltitude) {
			// 最大高度に達したらホーミングフェーズに移行
			phase_ = VerticalMissile::VerticalMissilePhase::HOMING;
		}

		break;
	case VerticalMissile::VerticalMissilePhase::HOMING:
	{

		// ターゲット位置を更新
		targetPos_ = ownerWeapon_->GetTargetPos();

		// ターゲット方向への単位ベクトルを計算
		Vector3 desired = targetPos_ - transform_.translate;
		desired = Vector3Math::Normalize(desired);

		// 現在の進行方向の単位ベクトルを計算
		Vector3 currentDir;
		if (velocity_.Length() > 0) {
			// 速度がある場合はその方向を使用
			currentDir = Vector3Math::Normalize(velocity_);
		} else {
			currentDir = desired; // 初回など速度が0なら目標方向へ
		}

		// homingRate_の値に基づいて徐々にターゲット方向に向かう
		float easedT = std::clamp(vmInfo_.homingRate, 0.0f, 1.0f);

		// 進行方向を補間して新しい方向を計算
		Vector3 mixed = Easing::Lerp(currentDir, desired, easedT);
		// 新しい方向ベクトルを計算
		direction_ = Vector3Math::Normalize(mixed);

		// 速度ベクトルを更新
		velocity_ = direction_ * speed_;
		// 位置を更新
		transform_.translate += velocity_ * deltaTime_;
	}
		break;
	case VerticalMissile::VerticalMissilePhase::EXPLODING:
		break;
	default:
		break;
	}

	//パーティクルエミッターの更新
	particleEmitter_[0]->SetTranslate(transform_.translate);
	particleEmitter_[0]->Update();

	particleEmitter_[1]->SetTranslate(transform_.translate);
	particleEmitter_[1]->Update();

	

	//オブジェクト、コライダーの更新
	object3d_->SetTranslate(transform_.translate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//MEMO: パーティクルの毎フレーム発生
	particleEmitter_[1]->Emit();
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("MissileSmoke")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("MissileExplosion")->SetEmitterPosition(transform_.translate);

}

void VerticalMissile::UpdateImGui() {}

//====================================================================================
// 描画処理
//====================================================================================

void VerticalMissile::Draw() {
	if (!isActive_) return; // 弾が無効化されている場合は描画しない
	object3d_->Draw();
}

void VerticalMissile::DrawCollider() {

	collider_->DrawCollider();
}

//====================================================================================
// 衝突判定の処理
//====================================================================================

void VerticalMissile::OnCollisionAction(GameCharacter* other) {

	//他のキャラクターと衝突した場合の処理
	if(characterType_ == CharacterType::PLAYER_MISSILE && other->GetCharacterType() == CharacterType::ENEMY ||
	   characterType_ == CharacterType::ENEMY_MISSILE && other->GetCharacterType() == CharacterType::PLAYER) {
		//パーティクル射出
		particleEmitter_[0]->Emit();
		//弾を無効化
		isActive_ = false; 
		//ポイントライト無効化
		pointLightData_.enabled_ = 0;
	}

	//レベルオブジェクトに当たった場合の処理
	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//パーティクル射出
		particleEmitter_[0]->Emit();
		//弾を無効化
		isActive_ = false; 
		//ポイントライト無効化
		pointLightData_.enabled_ = 0;
	}

}

//====================================================================================
// ミサイルの生成
//====================================================================================

void VerticalMissile::Create(BaseWeapon* ownerWeapon,VerticalMissileInfo vmInfo, float speed, float damage, CharacterType type) {

	ownerWeapon_ = ownerWeapon; // 所有者の武器を設定
	transform_.translate = ownerWeapon_->GetCenterPosition();
	characterType_ = type;
	speed_ = speed;
	vmInfo_ = vmInfo;
	damage_ = damage;
	targetPos_ = ownerWeapon_->GetTargetPos();
	//phaseの初期化	
	phase_ = VerticalMissilePhase::ASCENDING;
	//ターゲットまでの方向を求める
	direction_ = Vector3Math::Normalize(targetPos_ - transform_.translate);
	
	lifeTime_ = 5.0f; // ライフタイムを設定
	isActive_ = true;
	//ポイントライト有効化
	pointLightData_.enabled_ = 1;
}


//====================================================================================
// getter
//====================================================================================

//transformの取得
const EulerTransform& VerticalMissile::GetTransform() const { return transform_; }
//弾が有効かどうか
bool VerticalMissile::GetIsActive() const { return isActive_; }
//弾の速度の取得
const Vector3& VerticalMissile::GetVelocity() const { return velocity_; }
//ターゲットの座標の取得
const Vector3& VerticalMissile::GetTargetPos() const { return targetPos_; }
//攻撃力の取得
float VerticalMissile::GetDamage() const { return damage_; }
//弾速の取得
float VerticalMissile::GetSpeed() const { return speed_; }
//弾の半径の取得
float VerticalMissile::GetBulletRadius() const { return bulletRadius_; }
//寿命時間の取得
float VerticalMissile::GetLifeTime() const { return lifeTime_; }
//ミサイルのフェーズの取得
const VerticalMissile::VerticalMissilePhase& VerticalMissile::GetPhase() const { return phase_; }
