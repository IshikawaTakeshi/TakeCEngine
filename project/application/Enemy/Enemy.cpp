#define NOMINMAX
#include "Collision/CollisionTypeIdDef.h"
#include "Input.h"
#include "MatrixMath.h"
#include "Object3dCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Vector3Math.h"
#include "WinApp.h"

#include "Enemy.h"
#include "application/Enemy/EnemyBullet.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <numbers>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Enemy::~Enemy() {

	// 弾の解放
	for (auto& bullet : enemyBullet_) {
		delete bullet;
	}
	enemyBullet_.clear();
}

////////////////////////////////////////////////////////////////////////////////////
///		初期化処理
////////////////////////////////////////////////////////////////////////////////////

void Enemy::Initialize(Object3dCommon* object3dCommon, const std::string& filePath, Player* player) {

	player_ = player;

	//Audio読み込み
	throwRockSE = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/enemyThrowSE.wav");
	damageSE = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/enemyDamage.wav");

	// 衝突属性の設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemy));

	Object3d::Initialize(object3dCommon, filePath, true);

	transform_.translate = {0.0f, 4.0f, 20.0f};
	transform_.scale = {4.0f, 4.0f, 4.0f};

	isAlive_ = true;

	radius_ = 3.0f;

	hp_ = maxHP_;
	damageCoolTime_ = 60;

	deathTimer_ = 180;
	flyTimer_ = 180;

	phase_ = Phase::FIRST;

	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("Emitter_pBullet", transform_, 3, 0.15f);
	particleEmitter_->SetParticleName("Particle1");
	particleEmitter_->SetTranslate(transform_.translate);
	particleEmitter_->SetIsEmit(false);
}

////////////////////////////////////////////////////////////////////////////////////
///		更新処理
////////////////////////////////////////////////////////////////////////////////////

void Enemy::Update() {

	model_->GetMesh()->GetMaterial()->SetMaterialDataColor(color_);

	/*if (hp_ <= 0) {
		isAlive_ = false;
	}*/

	if (hp_ >= 0) {
		UpdatePhase();
		// 攻撃中の場合は攻撃処理を優先
		if (isAttack_) {
			Attack();
		} else {
			// 移動及び攻撃処理
			Move();
		}
	} else {

		if (deathTimer_ >= 0) {
			deathTimer_--;

			transform_.rotate = {0.0f, 0.0f, 0.0f};

			// 弾の更新は継続
			for (auto& bullet : enemyBullet_) {
				bullet->StraightRockUpdate();
				bullet->FallingRockUpdate();
			}

			// 元の位置を記録
			static Vector3 originalPosition = transform_.translate;

			// ランダムに揺れる範囲 (-0.1 ~ 0.1) を設定
			float shakeRange = 0.1f;
			float shakeX = (static_cast<float>(rand()) / RAND_MAX) * shakeRange * 2.0f - shakeRange;
			float shakeY = (static_cast<float>(rand()) / RAND_MAX) * shakeRange * 2.0f - shakeRange;
			float shakeZ = (static_cast<float>(rand()) / RAND_MAX) * shakeRange * 2.0f - shakeRange;

			// 震えを適用
			transform_.translate = originalPosition + Vector3{shakeX, shakeY, shakeZ};
		}
	}

	if (deathTimer_ <= 0) {
		
		transform_.rotate.x += 0.2f;
		transform_.translate.z += 0.2f;
		transform_.translate.y += 0.2f;

		flyTimer_--;
	}

	if (flyTimer_ <= 0) {
		isAlive_ = false;
	}

	// デスフラグの立った弾を削除
	enemyBullet_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	UpdateInvincibleTime();

	Object3d::Update();

	ImGuiDebug();

	particleEmitter_->SetTranslate(transform_.translate);
	particleEmitter_->Update();
}

////////////////////////////////////////////////////////////////////////////////////
///		描画処理
////////////////////////////////////////////////////////////////////////////////////

void Enemy::Draw() {

	for (auto& bullet : enemyBullet_) {
		bullet->Draw();
	}

	// モデル描画
	Object3d::Draw();
}

////////////////////////////////////////////////////////////////////////////////////
///		移動入力処理
////////////////////////////////////////////////////////////////////////////////////

void Enemy::Move() {

	velocity_ = {0, 0, 0};

#pragma region 1.旋回処理

#pragma endregion

#pragma region 2.移動処理

#pragma endregion

#pragma region 3.攻撃処理

	attackInterval_--;

	if (attackInterval_ <= 0) {
		isAttack_ = true;     // 攻撃を開始
		attackTime_ = 0;      // 攻撃時間を初期化
		attackInterval_ = 60; // 攻撃間隔をリセット
	}

#pragma endregion
}

void Enemy::UpdatePhase() {
	if (hp_ > 25) {
		phase_ = Phase::FIRST;
	} else if (hp_ > 15) {
		phase_ = Phase::SECOND;
	} else {
		phase_ = Phase::FINAL;
	}
}

const char* Enemy::PhaseToString(Phase phase) {

	switch (phase) {
	case Phase::FIRST:
		return "First";
	case Phase::SECOND:
		return "Second";
	case Phase::FINAL:
		return "Final";
	// 他の状態を追加
	default:
		return "Unknown";
	}
}

const char* Enemy::AttackTypeToString(AttackType attackType) {

	switch (attackType) {
	case AttackType::CHARGE:
		return "Charge";
	case AttackType::THROWROCK:
		return "ThrowRock";
	case AttackType::FALLINGROCK:
		return "FallingRock";
	// 他の状態を追加
	default:
		return "Unknown";
	}
}

void Enemy::Attack() {
	switch (phase_) {
	case Phase::FIRST:
		// 突進攻撃のみ
		currentAttackType_ = AttackType::CHARGE;
		ChargeAttack();
		break;

	case Phase::SECOND:
		// 突進攻撃と岩攻撃を交互に実行
		if (currentAttackType_ == AttackType::CHARGE) {
			ChargeAttack();
		} else {
			ThrowRockAttack();
		}
		break;

	case Phase::FINAL:

		// 全攻撃を交互に実行
		if (currentAttackType_ == AttackType::CHARGE) {
			ChargeAttack();
		} else if (currentAttackType_ == AttackType::THROWROCK) {
			ThrowRockAttack();
		} else {
			FallingRockAttack();
		}
		break;
	}
}

void Enemy::ChargeAttack() {

	const float attackDuration = 60.0f;   // 攻撃持続時間（フレーム）
	const float maxSpeed = 1.0f;          // 最大速度
	const float proportionalSpeed = 0.1f; // 差分に比例する速度係数
	const float minThreshold = 0.01f;     // しきい値

	// プレイヤーの位置を取得
	Vector3 playerPosition = player_->GetWorldPos();

	if (attackTime_ == 0) {
		// 突進開始時にプレイヤー方向のベクトルを計算
		Vector3 direction = playerPosition - transform_.translate;

		// ベクトルを正規化
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		if (length > 0.0f) {
			direction.x /= length;
			direction.y /= length;
			direction.z /= length;
		}

		// 最大速度を掛けたベクトルを設定
		velocity_ = direction * maxSpeed;
	}

	if (attackTime_ <= attackDuration) {
		// 突進処理
		transform_.translate += velocity_;
		attackTime_++;
	} else {
		// 突進後、初期位置に戻る処理
		Vector3 diff = transform_.translate - Vector3{0.0f, 4.0f, 20.0f};

		float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

		if (distance < minThreshold) {
			// 初期位置に近づいたら位置を固定し、攻撃終了
			transform_.translate = {0.0f, 4.0f, 20.0f};
			attackTime_ = 0;
			isAttack_ = false; // 攻撃終了
			ChangeAttackPattern();
		} else {
			// 差分の方向に比例した速度で戻る
			velocity_ = -diff * proportionalSpeed;
			transform_.translate += velocity_;
		}
	}
}

void Enemy::FallingRockAttack() {

	// 弾生成時の高さ
	const float spawnHeight = 15.0f;

	if (attackTime_ <= 300) {
		if (attackTime_ % 30 == 0) {
			for (int i = 0; i < 2; ++i) {
				Vector3 randomPosition = {
				    -15.0f + (std::rand() / (float)RAND_MAX) * 30.0f, // X座標: -15 から 15
				    spawnHeight,                                      // Y座標: 固定値
				    (std::rand() / (float)RAND_MAX) * -15.0f          // Z座標: 0 から 15
				};

				Vector3 bulletVelocity = {0.0f, -0.2f, 0.0f};

				EnemyBullet* newBullet = new EnemyBullet();
				newBullet->Initialize(Object3dCommon::GetInstance(), randomPosition, bulletVelocity);
				enemyBullet_.push_back(newBullet);
			}
		}
	} // else if (attackTime_ > 300 && attackTime_ <= 600) {

	//	// 弾の削除
	//	for (auto& bullet : enemyBullet_) {
	//		bullet->SetIsDead(true);
	//	}
	//}

	// 弾の更新
	for (auto& bullet : enemyBullet_) {
		bullet->FallingRockUpdate();
	}

	// 攻撃終了条件
	if (attackTime_ >= 600) {  // 合計10秒で攻撃終了
		isAttack_ = false;     // 攻撃終了
		ChangeAttackPattern(); // 攻撃パターン変更
	}

	attackTime_++;
}

void Enemy::ThrowRockAttack() {

	const float totalFrames = 480.0f;                                                           // 総フレーム数
	const float angleLimit = std::numbers::pi_v<float> / 4.0f;                                  // 回転角度の制限（±45度）
	const int oscillations = 3;                                                                 // 往復回数
	const float angularSpeed = (2.0f * oscillations * std::numbers::pi_v<float>) / totalFrames; // サイン波の角周波数

	// attackTime_に応じた回転角度の計算（ラジアン）
	float rotationAngle = angleLimit * sin(angularSpeed * attackTime_);
	transform_.rotate.y = rotationAngle;

	Vector3 velocity = {0.0f, 0.0f, -0.4f};

	Vector3 bulletPos = {transform_.translate.x, transform_.translate.y - 2.5f, transform_.translate.z};

	// 速度ベクトルを自機の向きに合わせて回転
	velocity = MatrixMath::TransformNormal(velocity, worldMatrix_);

	
	if (attackTime_ <= 480) {

		// 10フレームごとに弾の生成（調整可能）
		if (attackTime_ % 4 == 0) {
			
			EnemyBullet* newBullet = new EnemyBullet();
			newBullet->Initialize(Object3dCommon::GetInstance(), bulletPos, velocity);
			enemyBullet_.push_back(newBullet);
		}
		if (attackTime_ % 60 == 0) {
			AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), throwRockSE);
		}
	}

	// 弾の更新
	for (auto& bullet : enemyBullet_) {
		bullet->StraightRockUpdate();
	}

	// 攻撃終了条件
	if (attackTime_ >= 720) {
		isAttack_ = false;     // 攻撃終了
		ChangeAttackPattern(); // 攻撃パターン変更
	}

	attackTime_++;
}

void Enemy::ChangeAttackPattern() {

	// パターンを順番に切り替える例
	switch (currentAttackType_) {
	case AttackType::CHARGE:
		currentAttackType_ = AttackType::THROWROCK;
		break;
	case AttackType::THROWROCK:
		if (phase_ == Phase::SECOND) {
			currentAttackType_ = AttackType::CHARGE;
		} else {
			currentAttackType_ = AttackType::FALLINGROCK;
		}
		break;
	case AttackType::FALLINGROCK:
		currentAttackType_ = AttackType::CHARGE;
		break;
	}

	/*srand(static_cast<unsigned int>(time(nullptr)));
	int randomIndex = rand() % 2;
	currentAttackType_ = static_cast<AttackType>(randomIndex);*/
}

void Enemy::UpdateInvincibleTime() {
	if (isHit) {
		damageCoolTime_--;

		// 点滅処理
		if (damageCoolTime_ % 10 < 5) {
			color_ = {1.0f, 0.0f, 0.0f, 1.0f}; // 赤色
		} else {
			color_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 通常色
		}
	} else {
		damageCoolTime_ = 60;              // 被弾クールタイムをリセット
		color_ = {1.0f, 1.0f, 1.0f, 1.0f}; // 通常色に戻す
	}

	if (damageCoolTime_ <= 0) {
		isHit = false;
	}
}

////////////////////////////////////////////////////////////////////////////////////
///		ImGuiの更新処理
////////////////////////////////////////////////////////////////////////////////////

void Enemy::ImGuiDebug() {
#ifdef _DEBUG

	// playerの情報表示
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("translate", &transform_.translate.x, 0.01f);
	ImGui::DragInt("HP", &hp_);
	ImGui::DragInt("Damage Cool Time", &damageCoolTime_);
	ImGui::Checkbox("IsHit", &isHit);
	ImGui::Separator();
	ImGui::DragInt("attackInterval", &attackInterval_);
	ImGui::DragInt("attackTime", &attackTime_);
	ImGui::Separator();
	ImGui::Text("Current Attack Type: %s", AttackTypeToString(currentAttackType_));
	// 現在のPhaseを表示
	ImGui::Text("Current Phase: %s", PhaseToString(phase_));

	ImGui::ColorEdit4("color", &color_.x);

	ImGui::End();

#endif // DEBUG
}

////////////////////////////////////////////////////////////////////////////////////
///		衝突処理
////////////////////////////////////////////////////////////////////////////////////

void Enemy::OnCollision(Collider* other) {

	// 衝突相手の種別IDを取得
	uint32_t otherTypeID = other->GetTypeID();

	// 衝突相手がプレイヤーの場合
	if (otherTypeID == static_cast<uint32_t>(CollisionTypeIdDef::kPlayer)) {

	}

	// 衝突相手がプレイヤー弾の場合
	if (otherTypeID == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
		if (!isHit) {
			AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), damageSE);
			hp_--;
			isHit = true;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
///		プレイヤーのワールド座標の取得
////////////////////////////////////////////////////////////////////////////////////

Vector3 Enemy::GetWorldPos() {
	// ワールド行列の平行移動成分を取得(ワールド座標)
	Vector3 worldPos{worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2]};

	return worldPos;
}