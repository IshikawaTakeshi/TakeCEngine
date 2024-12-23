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
	Object3d::Initialize(object3dCommon, "lock.obj");

	transform_.translate = playerPosition;
	transform_.rotate = { 0.0f,0.0f,0.2f };
	//速度の初期化
	velocity_ = velocity;

	//生存フラグ
	isDead_ = false;

	// 重力の設定
	gravity_ = -0.02f; // 調整可能

	bulletfloor_ = -1.0f;//弾が跳ねる高さ

	//黄色に設定
	color_ = { 1.0f, 1.0f, 0.0f, 1.0f };
	model_->GetMesh()->GetMaterial()->SetMaterialDataColor(color_);
}

void PlayerBullet::Update() {


	//座標の移動
	transform_.translate += velocity_;



	//時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

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
