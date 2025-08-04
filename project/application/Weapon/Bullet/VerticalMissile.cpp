#include "VerticalMissile.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/MatrixMath.h"
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
	collider_->SetRadius(bulletradius_); // 半径を設定
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
	phase_ = VerticalMissilePhase::ASCENDING;

	speed_ = 140.0f; // 初期速度を設定
	lifeTime_ = 5.0f; // ライフタイムを設定
	bulletradius_ = 1.5f; // 弾の半径を設定
	homingRate_ = 0.4f;
}

//====================================================================================
// 更新処理
//====================================================================================

void VerticalMissile::Update() {

	//ライフタイムの減少
	lifeTime_ -= deltaTime_;
	if (lifeTime_ <= 0.0f || transform_.translate.y < 0.0f) {
		isActive_ = false;
	}

	switch (phase_) {
	case VerticalMissile::VerticalMissilePhase::ASCENDING:
		// 上昇中の処理
		transform_.translate.y += kAscendSpeed_ * TakeCFrameWork::GetDeltaTime();
		if (transform_.translate.y >= altitude_) {
			// 最大高度に達したらホーミングフェーズに移行
			transform_.translate.y = altitude_; // 高度を制限

			phase_ = VerticalMissile::VerticalMissilePhase::HOMING;
		}

		break;
	case VerticalMissile::VerticalMissilePhase::HOMING:

		targetPos_ = ownerWeapon_->GetTragetPos(); // ターゲット位置を更新
		direction_ = Vector3Math::Normalize(targetPos_ - transform_.translate);
		velocity_ = direction_ * (1.0f - homingRate_) * speed_;
		transform_.translate += velocity_ * TakeCFrameWork::GetDeltaTime();

		break;
	case VerticalMissile::VerticalMissilePhase::EXPLODING:
		break;
	default:
		break;
	}

	object3d_->SetTranslate(transform_.translate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//パーティクルエミッターの更新
	particleEmitter_[0]->SetTranslate(transform_.translate);
	particleEmitter_[0]->Update();

	particleEmitter_[1]->SetTranslate(transform_.translate);
	particleEmitter_[1]->Update();
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
	if (other->GetCharacterType() == CharacterType::PLAYER ||
		other->GetCharacterType() == CharacterType::ENEMY) {

		isActive_ = false; //弾を無効化
	}

	//レベルオブジェクトに当たった場合の処理
	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//パーティクル射出
		particleEmitter_[0]->Emit();

		isActive_ = false; //弾を無効化
	}

}

//====================================================================================
// ミサイルの生成
//====================================================================================

void VerticalMissile::Create(BaseWeapon* ownerWeapon, const float& speed, float damage, CharacterType type) {

	ownerWeapon_ = ownerWeapon; // 所有者の武器を設定
	transform_.translate = ownerWeapon_->GetCenterPosition();
	characterType_ = type;
	speed_ = speed;
	damage_ = damage;
	targetPos_ = ownerWeapon_->GetTragetPos();
	//ターゲットまでの方向を求める
	direction_ = Vector3Math::Normalize(targetPos_ - transform_.translate);
	altitude_ = transform_.translate.y + kMaxAltitude_; // 初期の上昇高度を設定
	isActive_ = true;
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
float VerticalMissile::GetBulletRadius() const { return bulletradius_; }
//寿命時間の取得
float VerticalMissile::GetLifeTime() const { return lifeTime_; }
//ミサイルのフェーズの取得
const VerticalMissile::VerticalMissilePhase& VerticalMissile::GetPhase() const { return phase_; }
