#include "PlayerBullet.h"
#include "TextureManager.h"
#include "Collision/CollisionTypeIdDef.h"
#include "TakeCFrameWork.h"

#include <cassert>
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

PlayerBullet::~PlayerBullet() {

}
void PlayerBullet::Initialize(Object3dCommon* object3dCommon, const Vector3& playerPosition, const Vector3& velocity) {

	//衝突属性の設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet));


	//オブジェクト情報の初期化
	Object3d::Initialize(object3dCommon,"sphere.obj");

	transform_.translate = playerPosition;
	transform_.rotate = { 0.0f,0.0f,0.2f };
	//速度の初期化
	velocity_ = velocity;

	//生存フラグ
	isDead_ = false;

	// 重力の設定
	gravity_ = -0.02f; // 調整可能

	bulletfloor_ = -2.0f;//弾が跳ねる高さ

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("Emitter_pBullet", transform_, 3, 0.15f);
	particleEmitter_->SetParticleName("PlayerBullet");
	particleEmitter_->SetTranslate(transform_.translate);
	particleEmitter_->SetFrequency(0.05f);
	particleEmitter_->SetParticleCount(5);
	particleEmitter_->SetIsEmit(true);
}

void PlayerBullet::Update() {

	//自機のバウンドする強さ
	const float bounceFactor = 0.8f;
	//速度に重力を＋
	velocity_.y += gravity_;
	//座標の移動
	transform_.translate += velocity_;

	// 地面に到達したときの跳ねる処理
	if (transform_.translate.y <= bulletfloor_) {
		transform_.translate.y = bulletfloor_; // 地面に位置を固定
		velocity_.y = -velocity_.y * bounceFactor; // Y軸の速度を反転し、反発係数を適用

		// 跳ねた回数をインクリメント
		bounceCount_++;

		// 最大跳ね回数に達した場合、弾を消す
		if (bounceCount_ >= kMaxBounces) {
			isDead_ = true;
		}
	}

	//時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	particleEmitter_->SetTranslate(transform_.translate);
	particleEmitter_->Update();

	//ワールド行列の更新
	Object3d::Update();
}

void PlayerBullet::UpdateImGui() {
#ifdef _DEBUG
	//playerBulletの情報表示
#endif // DEBUG
}

void PlayerBullet::Draw() {

	//モデル描画
	Object3d::Draw();
}

void PlayerBullet::OnCollision(Collider* other) {

	uint32_t otherTypeID = other->GetTypeID();

	//衝突相手が敵だったら
	if (otherTypeID == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy)) {
		isDead_ = true;
	}
}

Vector3 PlayerBullet::GetWorldPos() {

	// ワールド行列の平行移動成分を取得(ワールド座標)
	Vector3 worldPos{ worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2] };

	return worldPos;
}
