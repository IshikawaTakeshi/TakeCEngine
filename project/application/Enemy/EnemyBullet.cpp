#include "EnemyBullet.h"
#include "Collision/CollisionTypeIdDef.h"
#include "TextureManager.h"

#include <cassert>
#include <numbers>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

EnemyBullet::~EnemyBullet() {
}

void EnemyBullet::Initialize(Object3dCommon* object3dCommon, const Vector3& playerPosition, const Vector3& velocity) {


	// 衝突属性の設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet));

	// オブジェクト情報の初期化
	Object3d::Initialize(object3dCommon, "lock.obj");

	transform_.translate = playerPosition;
	// 速度の初期化
	velocity_ = velocity;

	transform_.scale = { 2.0f, 2.0f, 2.0f };

	// 生存フラグ
	isDead_ = false;

	// 重力を設定
	gravity_ = -0.05f; // 値を調整して放物線の形状を変化

	//緑色に設定
	color_ = { 0.2f, 1.0f, 0.2f, 1.0f };
	model_->GetMesh()->GetMaterial()->SetMaterialDataColor(color_);
}

void EnemyBullet::StraightRockUpdate() {


	transform_.translate += velocity_;

	// デス条件: 時間経過または特定のZ座標以下
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	// ワールド行列の更新
	Object3d::Update();
}


void EnemyBullet::Draw() {

	if (!isDead_) {
		// モデル描画
		Object3d::Draw();
	}
}

void EnemyBullet::OnCollision(Collider* other) {

	// 衝突相手の種別IDを取得
	const uint32_t typeID = other->GetTypeID();

	// 衝突相手がプレイヤーの場合
	if (typeID == static_cast<uint32_t>(CollisionTypeIdDef::kPlayer) ||
		typeID == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
		isDead_ = true;
	}
}

Vector3 EnemyBullet::GetWorldPos() {
	// ワールド行列の平行移動成分を取得(ワールド座標)
	Vector3 worldPos{ worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2] };

	return worldPos;
}

Vector3 EnemyBullet::GetTranslate() { return transform_.translate; }

const int32_t EnemyBullet::GetDeathTimer() const { return deathTimer_; }