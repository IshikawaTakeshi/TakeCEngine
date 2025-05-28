#include "Bullet.h"
#include "Collision/SphereCollider.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"
#include "math/Vector3Math.h"
#include "math/Easing.h"

//========================================================================================================
// object3d,colliderの初期化
//========================================================================================================

void Bullet::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<SphereCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	particleEmitters_.resize(5);
	//パーティクルエミッターの初期化
	for (int i = 0; i < particleEmitters_.size(); i++) {
		particleEmitters_[i] = std::make_unique<ParticleEmitter>();
	}

	speed_ = 500.0f;

}

//========================================================================================================
// 更新処理
//========================================================================================================

void Bullet::Update() {

	//移動処理
	//transform_.translate += velocity_ * deltaTime_;
	//弾の移動の補間
	transform_.translate = Easing::Lerp(transform_.translate, transform_.translate + velocity_, deltaTime_);

	//ライフタイムの減少
	lifeTime_ -= deltaTime_;
	if (lifeTime_ <= 0.0f) {
		isActive_ = false;
	}

	object3d_->SetTranslate(transform_.translate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//パーティクルエミッターの更新
	for (auto& emitter : particleEmitters_) {
		emitter->Update();
		emitter->SetTranslate(transform_.translate);
	}
}

void Bullet::UpdateImGui() {}

//========================================================================================================
// 描画処理
//========================================================================================================

void Bullet::Draw() {

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
			//敵のHPを減らす
			//enemy_->TakeDamage(damage_);

			//パーティクル射出
			for (auto& emitter : particleEmitters_) {
				
				emitter->SetIsEmit(true);
				emitter->Emit();
			}
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

}

//========================================================================================================
// 弾の座標、速度の初期化
//========================================================================================================

void Bullet::BulletInitialize(const Vector3& weaponPos,const Vector3& targetPos,CharacterType type) {

	transform_.translate = weaponPos;
	characterType_ = type;

	//ターゲットまでの方向を求める
	Vector3 direction = targetPos - transform_.translate;
	direction = Vector3Math::Normalize(direction);

	//速度の設定
	velocity_ = direction * speed_;
	lifeTime_ = 2.0f;
	isActive_ = true;
}
