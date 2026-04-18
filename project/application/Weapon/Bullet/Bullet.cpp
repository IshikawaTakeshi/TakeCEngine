#include "Bullet.h"
#include "Collision/SphereCollider.h"
#include "Collision/CollisionManager.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"
#include "math/Vector3Math.h"
#include "Math/Quaternion.h"
#include "math/Easing.h"

using namespace TakeC;

//========================================================================================================
// 初期化
//========================================================================================================
void Bullet::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//コライダー初期化
	collider_ = std::make_unique<SphereCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	
	collider_->SetOwner(this); // 持ち主を設定
	collider_->SetRadius(bulletRadius_); // 半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Bullet)); // 種別IDを設定
	
}

void Bullet::InitializeEffect(const BulletEffectConfig& effectConfig) {

	effectConfig_ = effectConfig;

	//effectGroupの初期化
	lightEffect_ = std::make_unique<EffectGroup>();
	lightEffect_->Initialize(effectConfig_.lightEffectFilePath);
	explosionEffect_ = std::make_unique<EffectGroup>();
	explosionEffect_->Initialize(effectConfig_.explosionEffectFilePath);
	
	//deltaTime取得
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
	//transform初期化
	transform_.translate = { 0.0f, 100.0f, 0.0f };
	//ポイントライト初期化
	pointLightData_.color_ = { 1.0f,0.3f,0.0f,1.0f };
	pointLightData_.intensity_ = 120.0f;
	pointLightData_.radius_ = 20.0f;
	pointLightData_.decay_ = 6.0f;
	//ポイントライトの追加
	pointLightIndex_ = TakeCFrameWork::GetLightManager()->AddPointLight(pointLightData_);
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Bullet::Update() {

	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	if (isActive_ == false) {
		pointLightData_.enabled_ = 0;

		lightEffect_->Stop();
		return;
	}

	//========================================================================
	// CCD (Continuous Collision Detection) - カプセル判定方式
	//========================================================================

	// 1フレームでの移動量を計算
	Vector3 displacement = velocity_ * deltaTime_;
	float moveDistance = Vector3Math::Length(displacement);

	bool isHit = false;

	// 移動量がある場合のみ判定
	if (moveDistance > 0.0001f) {
		// カプセルを構築（移動前→移動後）
		Capsule capsule;
		capsule.start = transform_.translate;           // 現在位置（移動前）
		capsule.end = transform_.translate + displacement; // 移動後の位置
		capsule.radius = bulletRadius_;                 // 弾の半径

		RayCastHit hitInfo;

		// 衝突対象のレイヤーマスクを設定
		uint32_t targetMask = 0;
		if (characterType_ == CharacterType::PLAYER_BULLET) {
			targetMask = static_cast<uint32_t>(CollisionLayer::Enemy);
		}
		else if (characterType_ == CharacterType::ENEMY_BULLET) {
			targetMask = static_cast<uint32_t>(CollisionLayer::Player);
		}

		// CapsuleCast実行
		if (CollisionManager::GetInstance().CapsuleCast(capsule, hitInfo, targetMask)) {
			// --- 衝突した場合 ---
			isHit = true;

			// 1. 弾の位置を衝突地点へ移動させる
			transform_.translate = hitInfo.position;

			// 2. 衝突相手のGameCharacterを取得して衝突処理を実行
			if (hitInfo.hitCollider) {
				GameCharacter* owner = hitInfo.hitCollider->GetOwner();
				if (owner) {
					OnCollisionAction(owner);
					owner->OnCollisionAction(this);
				}
			}
		}
	}

	// 衝突しなかった場合のみ、通常通り移動
	if (!isHit) {
		transform_.translate += displacement;
	}

	//ライフタイムの減少
	lifeTime_ -= deltaTime_;
	//ポイントライトの更新
	pointLightData_.position_ = transform_.translate;
	TakeCFrameWork::GetLightManager()->UpdatePointLight(pointLightIndex_, pointLightData_);

	//ライフタイムが0以下になったら弾を無効化
	if (lifeTime_ <= 0.0f) {
		pointLightData_.enabled_ = 0;
		pointLightData_.intensity_ = 0.0f;
		isActive_ = false;
		return;
	}

	object3d_->SetTranslate(transform_.translate);
	object3d_->SetRotate(transform_.rotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//effectGroupの更新
	lightEffect_->SetPosition(transform_.translate);
	lightEffect_->Update();
	explosionEffect_->Update();
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
		//敵に当たった場合の処理
		if (other->GetCharacterType() == CharacterType::ENEMY) {

			//パーティクル射出
			explosionEffect_->Play(transform_.translate);

			//ライトの点灯
			pointLightData_.enabled_ = 0;
			pointLightData_.intensity_ = 0.0f;
			isActive_ = false; //弾を無効化
		}
	} else if (characterType_ == CharacterType::ENEMY_BULLET) {
		//敵の弾の場合の処理
		if (other->GetCharacterType() == CharacterType::PLAYER) {

			//パーティクル射出
			explosionEffect_->Play(transform_.translate);
			//プレイヤーに当たった場合の処理
			pointLightData_.enabled_ = 0;
			pointLightData_.intensity_ = 0.0f;
			isActive_ = false; //弾を無効化
		}
	}

	//レベルオブジェクトに当たった場合の処理
	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//パーティクル射出
		explosionEffect_->Play(transform_.translate);
		pointLightData_.enabled_ = 0;
		pointLightData_.intensity_ = 0.0f;
		isActive_ = false; //弾を無効化
	}
}

//========================================================================================================
// 弾の座標、速度の初期化
//========================================================================================================

void Bullet::Create(const Vector3& weaponPos, const Vector3& targetPos,const Vector3& targetVel,float speed,float damage, CharacterType type) {

	transform_.translate = weaponPos;
	characterType_ = type;
	damage_ = damage;
	speed_ = speed;
	targetPos_ = targetPos;

	// 厳密迎撃予測
	float interceptTime = 0.0f;
	Vector3 aimPos = targetPos_;

	if (SolveInterceptTime(transform_.translate, targetPos_, targetVel, speed_, interceptTime)) {
		aimPos = targetPos_ + targetVel * interceptTime;
	}
	// 解けない場合は現地点を狙う（または max予測時間でクランプ）

	direction_ = Vector3Math::Normalize(aimPos - transform_.translate);

	Quaternion targetRotate = QuaternionMath::LookRotation(
		direction_, {0.0f, 1.0f, 0.0f});
	transform_.rotate = QuaternionMath::ToEuler(targetRotate);

	lifeTime_ = 2.0f;
	pointLightData_.enabled_ = 1;
	pointLightData_.intensity_ = 120.0f;

	velocity_ = direction_ * speed_;
	isActive_ = true;

	// パーティクルの再生
	lightEffect_->Play(transform_.translate);
}

void Bullet::Create(const Vector3& weaponPos, const Vector3& direction, float speed, float damage, CharacterType type) {

	transform_.translate = weaponPos;
	characterType_ = type;
	damage_ = damage;
	speed_ = speed;
	direction_ = Vector3Math::Normalize(direction);
	//ターゲットの方向にモデルを向ける
	float angle = std::atan2(direction_.x, direction_.z);
	transform_.rotate.y = angle;
	lifeTime_ = 2.0f; // 弾のライフタイムを設定
	pointLightData_.enabled_ = 1;
	pointLightData_.intensity_ = 120.0f;
	//速度の設定
	velocity_ = direction_ * speed_;
	isActive_ = true;

	// パーティクルの再生
	lightEffect_->Play(transform_.translate);
}

//========================================================================================================
// getter
//========================================================================================================

bool Bullet::SolveInterceptTime(const Vector3& shooterPos, const Vector3& targetPos, const Vector3& targetVel, float bulletSpeed, float& outTime) const {
	Vector3 r = targetPos - shooterPos;
	float a = Vector3Math::Dot(targetVel, targetVel) - bulletSpeed * bulletSpeed;
	float b = 2.0f * Vector3Math::Dot(r, targetVel);
	float c = Vector3Math::Dot(r, r);

	const float eps = 1e-5f;

	// a == 0 のときは一次方程式
	if (std::abs(a) < eps) {
		if (std::abs(b) < eps) return false; // 解なし
		float t = -c / b;
		if (t <= 0.0f) return false;
		outTime = t;
		return true;
	}

	float disc = b * b - 4.0f * a * c;
	if (disc < 0.0f) return false; // 実数解なし（追いつけない）

	float sqrtDisc = std::sqrt(disc);
	float t1 = (-b - sqrtDisc) / (2.0f * a);
	float t2 = (-b + sqrtDisc) / (2.0f * a);

	// 正の最小解を採用
	float t = FLT_MAX;
	if (t1 > 0.0f) t = t1;
	if (t2 > 0.0f && t2 < t) t = t2;

	if (t == FLT_MAX) return false;
	outTime = t;
	return true;
}

// Transformの取得
const EulerTransform& Bullet::GetTransform() const { return transform_; }
// 生存フラグの取得
bool Bullet::IsActive() { return isActive_; }
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