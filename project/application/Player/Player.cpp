#define NOMINMAX
#include "Player.h"
#include "Collision/CollisionTypeIdDef.h"
#include "Input.h"
#include "MatrixMath.h"
#include "Object3dCommon.h"
#include "PlayerBullet.h"
#include "Vector3Math.h"
#include "CameraManager.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "WinApp.h"

#include "PlayerState/PlayerStateIdle.h"
#include "PlayerState/PlayerStateRun.h"
#include "PlayerState/PlayerStateJump.h"
#include "PlayerState/PlayerStateAttack.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numbers>
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Player::~Player() {

	// 弾の解放
	for (auto& bullet : playerBullet_) {
		delete bullet;
	}
	playerBullet_.clear();
}

////////////////////////////////////////////////////////////////////////////////////
///		初期化処理
////////////////////////////////////////////////////////////////////////////////////

void Player::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	DamageSE = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/playerDamage.wav");

	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayer));
	Object3d::Initialize(object3dCommon, filePath);
	transform_.scale = {1.2f, 1.2f, 1.2f};
	transform_.translate = { 0.0f, 0.0f, -20.0f };
	isJumping_ = false;   // ジャンプ中かどうか
	jumpVelocity_ = 0.0f; // ジャンプの初速度
	playerfloor_ = 0.0f;  // 地面

	damageCoolTime_ = 60;
	hp_ = maxHP_;
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	// プレイヤーの状態を初期化
	SetState(std::make_unique<PlayerStateIdle>(this));
	state_->Initialize();

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("Emitter_pBullet", transform_, 3, 0.15f);
	particleEmitter_->SetParticleName("Particle1");

}

////////////////////////////////////////////////////////////////////////////////////
///		更新処理
////////////////////////////////////////////////////////////////////////////////////

void Player::Update() {

	// プレイヤーの状態を更新
	state_->Update();

	UpdateInvincibleTime();

	// 移動
	Move();

	// 攻撃
	Attack();

	// ジャンプ処理
	Jump();

	if (hp_ <= 0) {
		isAlive_ = false;
	}

	if (CameraManager::GetInstance()->GetActiveCamera()->GetIsShaking()) {
		shakingTime_--;
		CameraManager::GetInstance()->GetActiveCamera()->ShakeCamera();
	}

	if (shakingTime_ <= 0) {
		CameraManager::GetInstance()->GetActiveCamera()->SetIsShaking(false);
		shakingTime_ = 60;
	}

	Object3d::Update();

	particleEmitter_->SetTranslate(transform_.translate);
	particleEmitter_->Update();
}

////////////////////////////////////////////////////////////////////////////////////
///		描画処理
////////////////////////////////////////////////////////////////////////////////////

void Player::Draw() {

	// モデル描画
	Object3d::Draw();

}

void Player::DrawBullet() {
	// 弾の描画
	for (auto& bullet : playerBullet_) {
		bullet->Draw();
	}
}

void Player::DisPatch() {
	// ディスパッチ
	Object3d::DisPatch();
}

////////////////////////////////////////////////////////////////////////////////////
///		UI描画処理
////////////////////////////////////////////////////////////////////////////////////

void Player::DrawUI() {
	// 2Dレティクルの描画
	// sprite2DReticle_->Draw();
}

////////////////////////////////////////////////////////////////////////////////////
///		移動入力処理
////////////////////////////////////////////////////////////////////////////////////

void Player::Move() {

	velocity_ = { 0, 0, 0 };

#pragma region 1.旋回処理
	// 回転
	if (Input::GetInstance()->PushKey(DIK_LEFT)) {
		transform_.rotate.y -= 0.07f;
	} else if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		transform_.rotate.y += 0.07f;
	}

#pragma endregion

#pragma region 2.移動処理
	// 移動速度
	const float moveSpeed = 0.5f;

	// ゲームパッドの状態を取得
	if (Input::GetInstance()->PushKey(DIK_A)) {
		velocity_.x -= moveSpeed;
	} else if (Input::GetInstance()->PushKey(DIK_D)) {
		velocity_.x += moveSpeed;
	}

	if (Input::GetInstance()->PushKey(DIK_W)) {
		velocity_.z += moveSpeed;
	} else if (Input::GetInstance()->PushKey(DIK_S)) {
		velocity_.z -= moveSpeed;
	}

	// 座標移動(ベクトルの加算)
	transform_.translate += velocity_;

	// 移動範囲の制限
	transform_.translate.x = std::clamp(transform_.translate.x, -kMoveLimit_.x, kMoveLimit_.x);
	transform_.translate.z = std::clamp(transform_.translate.z, -kMoveLimit_.y, 0.0f);
#pragma endregion



}

void Player::SetState(std::unique_ptr<BasePlayerState> state) {
	state_ = std::move(state);
	state_->Initialize();
}

////////////////////////////////////////////////////////////////////////////////////
///		ImGuiの更新処理
////////////////////////////////////////////////////////////////////////////////////

void Player::UpdateImGui() {
#ifdef _DEBUG

	// playerの情報表示
	ImGui::Begin("Player");

	ImGui::DragFloat3("scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("translate", &transform_.translate.x, 0.01f);
	ImGui::DragInt("shakingTime", &shakingTime_);
	ImGui::Text("State: %s", state_->GetName().c_str());
	//Object3d::UpdateImGui(12);
	ImGui::End();

#endif // DEBUG
}

////////////////////////////////////////////////////////////////////////////////////
///		衝突処理
////////////////////////////////////////////////////////////////////////////////////

void Player::OnCollision(Collider* other) {

	// 衝突相手の種別IDを取得
	uint32_t otherTypeID = other->GetTypeID();

	// 衝突相手が敵の場合
	if (otherTypeID == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy)) {
		if (!isHit) {
			AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), DamageSE,1.1f);
			CameraManager::GetInstance()->GetActiveCamera()->SetShake(60.0f, 0.2f);
			CameraManager::GetInstance()->GetActiveCamera()->SetIsShaking(true);
			hp_--;
			isHit = true;
		}
	}

	// 衝突相手が敵弾の場合
	if (otherTypeID == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) {
		if (!isHit) {
			AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), DamageSE,1.1f);
			CameraManager::GetInstance()->GetActiveCamera()->SetShake(60.0f, 0.2f);
			CameraManager::GetInstance()->GetActiveCamera()->SetIsShaking(true);
			hp_--;
			isHit = true;
		}
	}
}

void Player::UpdateInvincibleTime() {
	if (isHit) {
		damageCoolTime_--;

		// 点滅処理
		if (damageCoolTime_ % 10 < 5) {
			color_ = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤色
		} else {
			color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 通常色
		}
	} else {
		damageCoolTime_ = 60;              // 被弾クールタイムをリセット
		color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 通常色に戻す
	}

	if (damageCoolTime_ <= 0) {
		isHit = false;
	}
}

////////////////////////////////////////////////////////////////////////////////////
///		プレイヤーのワールド座標の取得
////////////////////////////////////////////////////////////////////////////////////

Vector3 Player::GetWorldPos() {
	// ワールド行列の平行移動成分を取得(ワールド座標)
	Vector3 worldPos{ worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2] };

	return worldPos;
}

////////////////////////////////////////////////////////////////////////////////////
///		照準オブジェクトのワールド座標の取得
////////////////////////////////////////////////////////////////////////////////////

Vector3 Player::GetWorldPos3DReticle() {
	/*Vector3 worldPos{
		worldTransform3DReticle_.matWorld_.m[3][0],
		worldTransform3DReticle_.matWorld_.m[3][1],
		worldTransform3DReticle_.matWorld_.m[3][2]
	};

	return worldPos;*/

	return { 0, 0, 0 };
}

////////////////////////////////////////////////////////////////////////////////////
///		攻撃処理
////////////////////////////////////////////////////////////////////////////////////

void Player::Attack() {


	attackInterval_++;
	if (attackInterval_ > 20) {
		if (Input::GetInstance()->IsTriggerMouse(0)) {
			ShotBullet();
		}
	}

	// 弾の更新
	for (auto& bullet : playerBullet_) {
		bullet->Update();
	}

	// デスフラグの立った弾を削除
	playerBullet_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
		});
}

void Player::ShotBullet() {

	// 弾の速度
	const float kBulletSpeedZ = 1.44f;
	const float kBulletSpeedY = 0.44f;

	Vector3 bulletVelocity = { 0, kBulletSpeedY, kBulletSpeedZ };

	// プレイヤーから照準オブジェクトへのベクトル
	bulletVelocity = MatrixMath::TransformNormal(bulletVelocity, worldMatrix_);

	// 弾の生成
	PlayerBullet* newBullet = new PlayerBullet();
	newBullet->Initialize(Object3dCommon::GetInstance(), GetWorldPos(), bulletVelocity);
	// newBullet->SetParent(&worldTransform_); //親子関係の設定(ワールド行列の更新を親に依存する
	// 弾を登録する
	playerBullet_.push_back(newBullet);

	attackInterval_ = 0;

}

void Player::Jump() {

	if (isJumping_) {
		jumpVelocity_ += gravity_; // 重力を加算して下降
		transform_.translate.y += jumpVelocity_;

		// 地面に着地した場合
		if (transform_.translate.y <= playerfloor_) {
			transform_.translate.y = playerfloor_;
			isJumping_ = false;
			jumpVelocity_ = 0.0f;
		}
	} else {
		// スペースキーでジャンプを開始
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			if (!isJumping_) {
				isJumping_ = true;
				jumpVelocity_ = jumpForce_; // ジャンプの初速度を設定
			}
		}
	}
}
