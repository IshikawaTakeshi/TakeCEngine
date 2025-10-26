#include "Bullet.h"
#include "Collision/SphereCollider.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"
#include "math/Vector3Math.h"
#include "math/Easing.h"

//========================================================================================================
// 初期化
//========================================================================================================
void Bullet::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

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
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Bullet)); // 種別IDを設定
	//emiiter設定
	//emitter0
	particleEmitter_.resize(2);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 5, 0.001f);
	particleEmitter_[0]->SetParticleName("SparkExplosion");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 8, 0.001f);
	particleEmitter_[1]->SetParticleName("SmokeEffect");

	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	lifeTime_ = 2.0f; // 弾のライフタイムを設定

	transform_.translate = { 0.0f, 100.0f, 0.0f };
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Bullet::Update() {

	//移動処理
	transform_.translate += velocity_ * deltaTime_;

	//ライフタイムの減少
	lifeTime_ -= deltaTime_;
	if (lifeTime_ <= 0.0f) {
		isActive_ = false;
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
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SparkExplosion")->SetEmitterPosition(transform_.translate);

}

//========================================================================================================
// ImGuiの更新
//========================================================================================================
void Bullet::UpdateImGui() {

}

//========================================================================================================
// 描画処理
//========================================================================================================

void Bullet::Draw() {
	if (!isActive_) return; // 弾が無効化されている場合は描画しない
	object3d_->Draw();
}

//========================================================================================================
// 衝突判定の描画
//========================================================================================================

void Bullet::DrawCollider() {

	collider_->DrawCollider();
}

//========================================================================================================
// 衝突判定の処理
//========================================================================================================

void Bullet::OnCollisionAction(GameCharacter* other) {

	if (characterType_ == CharacterType::PLAYER_BULLET) {
		if (other->GetCharacterType() == CharacterType::ENEMY) {
			//敵に当たった場合の処理

			//パーティクル射出
			//particleEmitter_[0]->Emit();
			isActive_ = false; //弾を無効化
		}
	} else if (characterType_ == CharacterType::ENEMY_BULLET) {
		//敵の弾の場合の処理
		if (other->GetCharacterType() == CharacterType::PLAYER) {
			//プレイヤーに当たった場合の処理

			//player_->TakeDamage(damage_);
			isActive_ = false; //弾を無効化
		}
	}

	//レベルオブジェクトに当たった場合の処理
	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//パーティクル射出
		//particleEmitter_[0]->Emit();

		isActive_ = false; //弾を無効化
	}
}

//========================================================================================================
// 弾の座標、速度の初期化
//========================================================================================================

void Bullet::Create(const Vector3& weaponPos, const Vector3& targetPos,float speed,float damage, CharacterType type) {

	transform_.translate = weaponPos;
	characterType_ = type;
	damage_ = damage;
	speed_ = speed;
	targetPos_ = targetPos;
	//ターゲットまでの方向を求める
	direction_ = Vector3Math::Normalize(targetPos_ - transform_.translate);

	//速度の設定
	velocity_ = direction_ * speed_;
	isActive_ = true;
}

//========================================================================================================
// getter
//========================================================================================================

// Transformの取得
const EulerTransform& Bullet::GetTransform() const { return transform_; }
// 生存フラグの取得
bool Bullet::GetIsActive() { return isActive_; }
// 速度の取得
const Vector3& Bullet::GetVelocity() const { return velocity_; }
// ターゲット位置の取得
const Vector3& Bullet::GetTargetPos() const { return targetPos_; }
// 攻撃力の取得
float Bullet::GetDamage() const { return damage_; }
// 弾速の取得
float Bullet::GetSpeed() const { return speed_; }
// 弾の半径の取得
float Bullet::GetBulletRadius() const { return bulletRadius_; }
// 寿命時間の取得
float Bullet::GetLifeTime() const { return lifeTime_; }

//========================================================================================================
// setter
//========================================================================================================

	// 生存フラグを設定
void Bullet::SetIsActive(bool isActive) { isActive_ = isActive; }

// 速度を設定
void Bullet::SetVelocity(const Vector3& velocity) { velocity_ = velocity; }

// ターゲット位置を設定
void Bullet::SetTargetPos(const Vector3& targetPos) { targetPos_ = targetPos; }

// 弾の半径を設定
void Bullet::SetBulletRadius(float radius) {
	bulletRadius_ = radius;
	// コライダーの半径も更新
	collider_->SetRadius(bulletRadius_);
}

// 弾速を設定
void Bullet::SetSpeed(float speed) { speed_ = speed; }

// 攻撃力を設定
void Bullet::SetDamage(float damage) { damage_ = damage; }