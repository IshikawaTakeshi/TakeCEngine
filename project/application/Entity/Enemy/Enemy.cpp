#include "Enemy.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Model.h"
#include "engine/io/Input.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "Vector3Math.h"
#include "math/Easing.h"

#include "application/Weapon/Rifle.h"
#include "application/Weapon/Bazooka.h"
#include "application/Entity/WeaponUnit.h"

Enemy::~Enemy() {
	object3d_.reset();
	collider_.reset();
}

//========================================================================================================
//　初期化
//========================================================================================================


void Enemy::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {


	//キャラクタータイプ設定
	characterType_ = CharacterType::ENEMY;

	behaviorRequest_ = Behavior::RUNNING;
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetHalfSize({ 2.0f, 2.5f, 2.0f }); // コライダーの半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Enemy));

	transform_ = { {1.5f,1.5f,1.5f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,30.0f} };
	object3d_->SetScale(transform_.scale);

	//emiiter設定
	//emitter0
	particleEmitter_.resize(3);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0",{ {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 5, 1.0f);
	particleEmitter_[0]->SetParticleName("DamageSpark");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 10, 1.0f);
	particleEmitter_[1]->SetParticleName("CrossEffect");
	//emitter2
	particleEmitter_[2] = std::make_unique<ParticleEmitter>();
	particleEmitter_[2]->Initialize("EnemyEmitter2", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 10, 1.0f);
	particleEmitter_[2]->SetParticleName("SparkExplosion");

	weapons_.resize(2); // 武器の数を2つに設定
	weaponTypes_.resize(2);
	weaponTypes_[R_ARMS] = WeaponType::WEAPON_TYPE_RIFLE; // 1つ目の武器はライフル
	weaponTypes_[L_ARMS] = WeaponType::WEAPON_TYPE_BAZOOKA; // 2つ目の武器はバズーカ

	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	//体力の初期化
	health_ = maxHealth_;
}

void Enemy::WeaponInitialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {
	//武器の初期化
	for (int i = 0; i < weapons_.size(); i++) {
		if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_RIFLE) {
			weapons_[i] = std::make_unique<Rifle>();
			weapons_[i]->Initialize(object3dCommon, bulletManager, "Rifle.gltf");
			weapons_[i]->SetOwnerObject(this);
		}else if(weaponTypes_[i] == WeaponType::WEAPON_TYPE_BAZOOKA) {
			weapons_[i] = std::make_unique<Bazooka>();
			weapons_[i]->Initialize(object3dCommon, bulletManager, "Bazooka.gltf");
			weapons_[i]->SetOwnerObject(this);
		}
	}

	weapons_[R_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "RightHand"); // 1つ目の武器を右手に取り付け
	weapons_[L_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "LeftHand"); // 2つ目の武器を左手に取り付け
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Enemy::Update() {

	// ランダムエンジンの初期化  
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	//stepBoostのインターバルの更新
	if(stepBoostIntervalTimer_ > 0.0f) {
		stepBoostIntervalTimer_ -= deltaTime_;
	}

	// StepBoost入力判定を最初に追加
	if (behavior_ == Behavior::RUNNING) {

	
			//1秒に1度確率で判定
			if( randomEngine() % 100 < jumpProbability_) { // 1%の確率
				if(randomEngine() % 2 == 0) {
					//Jump遷移
					behaviorRequest_ = Behavior::JUMP;
				} else {
					//StepBoost遷移
					behaviorRequest_ = Behavior::STEPBOOST;
				}
			}
	}

	//エネルギーの更新
	UpdateEnergy();

	if (behaviorRequest_) {

		behavior_ = behaviorRequest_.value();
		prevBehavior_ = behavior_;

		switch (behavior_) {
		case Behavior::IDLE:
			break;
		case Behavior::RUNNING:
			InitRunning();
			break;
		case Behavior::JUMP:
			InitJump();
			break;
		case Behavior::DASH:
			InitDash();
			break;
		case Behavior::STEPBOOST:
			InitStepBoost();
			break;
		case Behavior::FLOATING:
			InitFloating();
			break;
		case Behavior::CHARGESHOOT:
			InitChargeShoot();
			break;
		case Behavior::CHARGESHOOT_STUN:
			InitChargeShootStun();
			break;
		case Behavior::DEAD:
			InitDead();
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

	switch (behavior_) {
	case Enemy::Behavior::IDLE:
		break;
	case Enemy::Behavior::RUNNING:
		UpdateRunning();
		break;
	case Enemy::Behavior::JUMP:
		UpdateJump();
		break;
	case Enemy::Behavior::DASH:
		UpdateDash();
		break;
	case Enemy::Behavior::STEPBOOST:
		UpdateStepBoost();
		break;
	case Enemy::Behavior::FLOATING:
		UpdateFloating(randomEngine);
		break;
	case Enemy::Behavior::CHARGESHOOT:
		UpdateChargeShoot();
		break;
	case Enemy::Behavior::CHARGESHOOT_STUN:
		UpdateChargeShootStun();
		break;

	case Enemy::Behavior::DEAD:
		UpdateDead();
		break;

	default:
		break;
	}

	//攻撃処理
	if (isAlive_ == true) {
		UpdateAttack();
	}

	//ダメージエフェクトの更新
	if (isDamaged_) {
		damageEffectTime_ -= deltaTime_;
		particleEmitter_[0]->Emit();
		if (damageEffectTime_ <= 0.0f) {
			isDamaged_ = false;
		}
	}

	if(health_ <= 0.0f) {
		//死亡状態のリクエスト
		isAlive_ = false;
		behaviorRequest_ = Behavior::DEAD;
	}

	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::toEuler(transform_.rotate);
	object3d_->SetTranslate(transform_.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	weapons_[0]->SetTarget(focusTargetPos_);
	weapons_[0]->Update();
	weapons_[1]->SetTarget(focusTargetPos_);
	weapons_[1]->Update(); 

	//パーティクルエミッターの更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(transform_.translate);
		emitter->Update();
	}
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DamageSpark")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SparkExplosion")->SetEmitterPosition(transform_.translate);
}

void Enemy::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &health_, 1.0f, 0.0f, maxHealth_);
	ImGui::DragFloat("Energy", &energy_, 1.0f, 0.0f, maxEnergy_);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &moveDirection_.x, 0.01f);
	ImGui::Text("Behavior: %d", static_cast<int>(behavior_));
	object3d_->UpdateImGui("Enemy");
	ImGui::End();
#endif // _DEBUG
}

//========================================================================================================
// 描画処理
//========================================================================================================


void Enemy::Draw() {

	object3d_->Draw();
	for(const auto& weapon : weapons_) {
		weapon->Draw();
	}

}

void Enemy::DrawCollider() {

#ifdef _DEBUG
	collider_->DrawCollider();

#endif // _DEBUG

}

//========================================================================================================
// 当たり判定の処理
//========================================================================================================

void Enemy::OnCollisionAction(GameCharacter* other) {

	if (other->GetCharacterType() == CharacterType::PLAYER) {

	}

	if (other->GetCharacterType()  == CharacterType::PLAYER_BULLET) {

		Bullet* bullet = dynamic_cast<Bullet*>(other);
		//プレイヤーの弾に当たった場合の処理
		particleEmitter_[1]->Emit();
		isDamaged_ = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		health_ -= bullet->GetDamage();
	}
	if(other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
		//プレイヤーのミサイルに当たった場合の処理
		//particleEmitter_[2]->Emit();
		isDamaged_ = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		health_ -= missile->GetDamage();
	}
}

//===================================================================================
//　走行処理
//===================================================================================

void Enemy::InitRunning() {

}


void Enemy::UpdateRunning() {

	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(focusTargetPos_ - transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	// 回転補間
	transform_.rotate = Easing::Slerp(transform_.rotate, targetRotation, followSpeed_);

	// 移動方向の計算
	// ターゲットの周囲を回るための目標座標を計算
	orbitAngle_ += orbitSpeed_ * deltaTime_;  // 時間で角度加算
	if (orbitAngle_ > 2 * std::numbers::pi_v<float>) orbitAngle_ -= 2 * std::numbers::pi_v<float>;
	// ターゲットの周囲を回る座標を計算（XZ平面で円運動）
	Vector3 orbitPos;
	orbitPos.x = focusTargetPos_.x + orbitRadius_ * cos(orbitAngle_);
	orbitPos.y = focusTargetPos_.y; // 高さはターゲットに合わせる（必要に応じて調整）
	orbitPos.z = focusTargetPos_.z + orbitRadius_ * sin(orbitAngle_);
	// 目的座標までの方向ベクトルを計算
	toOrbitPos_ = orbitPos - transform_.translate;


	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
		//移動方向の正規化
		moveDirection_ = Vector3Math::Normalize(toOrbitPos_);
		//移動時の加速度の計算
		velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
		velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;

		float targetAngle = atan2(moveDirection_.x, moveDirection_.z);
		Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, targetAngle);

		transform_.rotate = Easing::Slerp(transform_.rotate, targetRotate, 0.1f);
		transform_.rotate = QuaternionMath::Normalize(transform_.rotate);

	} else {
		//速度の減速処理
		velocity_.x /= deceleration_;
		velocity_.z /= deceleration_;
	}

	//最大移動速度の制限
	float speed = sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speed > kMaxMoveSpeed_) {
		float scale = kMaxMoveSpeed_ / speed;
		velocity_.x *= scale;
		velocity_.z *= scale;
	}

	//移動処理
	// 位置の更新（deltaTimeをここで適用）
	transform_.translate.x += velocity_.x * deltaTime_;
	transform_.translate.z += velocity_.z * deltaTime_;
}

//===================================================================================
//　ジャンプ処理
//===================================================================================

void Enemy::InitJump() {

	//オーバーヒート状態のチェック
	if (isOverheated_) {
		// オーバーヒート中はジャンプできない
		behaviorRequest_ = Behavior::RUNNING;
		return;
	}

	// ジャンプのエネルギー消費
	energy_ -= useEnergyJump_;

	//ジャンプの初期化
	//ジャンプの速度を設定
	velocity_.y = jumpSpeed_;

	//ジャンプ中の移動方向を設定
	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(focusTargetPos_ - transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	// 回転補間
	transform_.rotate = Easing::Slerp(transform_.rotate, targetRotation, followSpeed_);

	// ターゲットの周囲を回るための目標座標を計算
	orbitAngle_ += orbitSpeed_ * deltaTime_;
	if (orbitAngle_ > 2 * std::numbers::pi_v<float>) orbitAngle_ -= 2 * std::numbers::pi_v<float>;

	Vector3 orbitPos;
	orbitPos.x = focusTargetPos_.x + orbitRadius_ * cos(orbitAngle_);
	orbitPos.y = focusTargetPos_.y; // 高さはターゲットに合わせる（必要なら補正可）
	orbitPos.z = focusTargetPos_.z + orbitRadius_ * sin(orbitAngle_);

	// 目的座標までの方向ベクトル
	toOrbitPos_ = orbitPos - transform_.translate;

	// 移動方向の決定（XZのみでfloat時はほぼ水平移動を想定）
	moveDirection_ = Vector3Math::Normalize(Vector3(toOrbitPos_.x, 0.0f, toOrbitPos_.z));
	
	transform_.translate.y += 0.1f; // 少し上に移動してジャンプ感を出す
}

void Enemy::UpdateJump() {
	// ジャンプ中の移動
	transform_.translate.x += velocity_.x * deltaTime_;
	transform_.translate.z += velocity_.z * deltaTime_;
	// 重力の適用
	velocity_.y -= (gravity_ + jumpDeceleration_) * deltaTime_;
	transform_.translate.y += velocity_.y * deltaTime_;

	jumpTimer_ += deltaTime_;
	if (jumpTimer_ > maxJumpTime_) {
		behaviorRequest_ = Behavior::FLOATING;
		return;
	}

	// 地面に着地したらRUNNINGに戻る
	if (transform_.translate.y <= 0.0f) {
		transform_.translate.y = 0.0f; // 地面に合わせる
		behaviorRequest_ = Behavior::RUNNING;
		velocity_ = { 0.0f, 0.0f, 0.0f }; // ジャンプ中の速度をリセット
	}
}

//===================================================================================
//　ダッシュ処理
//===================================================================================

void Enemy::InitDash() {}

void Enemy::UpdateDash() {

}

//===================================================================================
// 攻撃処理
//===================================================================================


void Enemy::UpdateAttack() {

	for (int i = 0; i < 1; ++i) {
		auto* weapon = weapons_[i].get();

		// AI判定で攻撃開始（チャージor通常）
		if (ShouldStartAttack(i)) {
			if (weapon->IsChargeAttack()) {
				// チャージ開始
				weapon->Charge(deltaTime_);
				if (ShouldReleaseAttack(i)) {
					// チャージ攻撃実行
					weapon->ChargeAttack();
					if (weapon->IsStopShootOnly()) {
						behaviorRequest_ = Behavior::CHARGESHOOT_STUN;
					} else {
						behaviorRequest_ = Behavior::RUNNING;
					}
				}
			} else {
				// 通常攻撃
				weapon->Attack();
				if (weapon->IsStopShootOnly()) {
					behaviorRequest_ = Behavior::CHARGESHOOT_STUN;
				}
				// 移動撃ちならRUNNING継続
			}
		} else if (ShouldReleaseAttack(i)) {
			// 「離す」判定（チャージ終わり）
			if (weapon->IsCharging()) {
				weapon->ChargeAttack();
				if (weapon->IsStopShootOnly()) {
					behaviorRequest_ = Behavior::CHARGESHOOT_STUN;
				} else {
					behaviorRequest_ = Behavior::RUNNING;
				}
			}
		}
	}
}


//===================================================================================
//　チャージ攻撃処理
//===================================================================================
void Enemy::InitChargeShoot() {

}
void Enemy::UpdateChargeShoot() {

	// 速度を大きく落としていき、停止させた後攻撃処理を入れる
	velocity_.x *= 0.5f;
	velocity_.z *= 0.5f;
	if (std::abs(velocity_.x) < 0.01f && std::abs(velocity_.z) < 0.01f) {
		// チャージ攻撃の実行
		for (auto& weapon : weapons_) {
			if (weapon->IsChargeAttack()) {
				weapon->ChargeAttack();

			}else {
				// チャージ攻撃不可の場合は通常攻撃
				weapon->Attack();
			}

			if (weapon->IsMoveShootable()) {
				// ステップ終了時前回の状態に戻す
				if (transform_.translate.y <= 0.0f) {
					behaviorRequest_ = Behavior::RUNNING;
				} else if (transform_.translate.y > 0.0f) {
					behaviorRequest_ = Behavior::FLOATING;
				} else {
					// ステップブーストが終了したらRUNNINGに戻す
					behaviorRequest_ = Behavior::RUNNING;
				}
			} else {
				// 停止撃ち専用の場合はCHARGESHOOT_STUNに切り替え
				behaviorRequest_ = Behavior::CHARGESHOOT_STUN;
			}
		}
	}

	// 位置の更新（deltaTimeをここで適用）
	transform_.translate.x += velocity_.x * deltaTime_;
	transform_.translate.z += velocity_.z * deltaTime_;
}

//===================================================================================
//　チャージ攻撃後の硬直処理
//===================================================================================

void Enemy::InitChargeShootStun() {
	chargeAttackStunTimer_ = chargeAttackStunDuration_;
}

void Enemy::UpdateChargeShootStun() {

	chargeAttackStunTimer_ -= deltaTime_;
	if (chargeAttackStunTimer_ <= 0.0f) {
		// ステップ終了時前回の状態に戻す
		if (transform_.translate.y <= 0.0f) {
			behaviorRequest_ = Behavior::RUNNING;
		} else if (transform_.translate.y > 0.0f) {
			behaviorRequest_ = Behavior::FLOATING;
		} else {
			// ステップブーストが終了したらRUNNINGに戻す
			behaviorRequest_ = Behavior::RUNNING;
		}
	}
}

//===================================================================================
//　ステップブースト処理
//===================================================================================


void Enemy::InitStepBoost() {

	// オーバーヒート状態のチェック
	if (isOverheated_) {
		// オーバーヒート中はステップブーストできない
		behaviorRequest_ = Behavior::RUNNING;
		return;
	}

	// ステップブーストのエネルギーを消費
	energy_ -= useEnergyStepBoost_;

	//上昇速度を急激に遅くする
	velocity_.y = 0.0f;

	//ジャンプ中の移動方向を設定
	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(focusTargetPos_ - transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	// 回転補間
	transform_.rotate = Easing::Slerp(transform_.rotate, targetRotation, followSpeed_);

	// ターゲットの周囲を回るための目標座標を計算
	orbitAngle_ += orbitSpeed_ * deltaTime_;
	if (orbitAngle_ > 2 * std::numbers::pi_v<float>) orbitAngle_ -= 2 * std::numbers::pi_v<float>;

	Vector3 orbitPos;
	orbitPos.x = focusTargetPos_.x + orbitRadius_ * cos(orbitAngle_);
	orbitPos.y = focusTargetPos_.y; // 高さはターゲットに合わせる（必要なら補正可）
	orbitPos.z = focusTargetPos_.z + orbitRadius_ * sin(orbitAngle_);

	// 目的座標までの方向ベクトル
	toOrbitPos_ = orbitPos - transform_.translate;

	// 移動方向の決定（XZのみでfloat時はほぼ水平移動を想定）
	stepBoostDirection_ = Vector3Math::Normalize(Vector3(toOrbitPos_.x, 0.0f, toOrbitPos_.z));

	stepBoostTimer_ = stepBoostDuration_;
	velocity_.x = stepBoostDirection_.x * stepBoostSpeed_;
	velocity_.z = stepBoostDirection_.z * stepBoostSpeed_;
}

void Enemy::UpdateStepBoost() {
	// ステップ中の移動
	transform_.translate.x += velocity_.x * deltaTime_;
	transform_.translate.z += velocity_.z * deltaTime_;

	stepBoostTimer_ -= deltaTime_;
	if (stepBoostTimer_ <= 0.0f) {
		// ステップ終了時前回の状態に戻す
		if (transform_.translate.y <= 0.0f) {
			behaviorRequest_ = Behavior::RUNNING;
		} else if (transform_.translate.y > 0.0f) {
			behaviorRequest_ = Behavior::FLOATING;
		} else {
			// ステップブーストが終了したらRUNNINGに戻す
			behaviorRequest_ = Behavior::RUNNING;
		}

		// ステップブーストのインターバルをリセット
		stepBoostIntervalTimer_ = stepBoostInterval_;
	}
}

void Enemy::TriggerStepBoost() {
	if (stepBoostIntervalTimer_ <= 0.0f) {
		StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
		if (fabs(leftStick.x) > 0.2f || fabs(leftStick.y) > 0.2f) {
			//方向ベクトル計算（カメラ考慮）

			stepBoostDirection_ = Vector3Math::Normalize(toOrbitPos_);
			behaviorRequest_ = Behavior::STEPBOOST;
		}
	}
}



//===================================================================================
// 浮遊時の処理
//===================================================================================

void Enemy::InitFloating() {

}

void Enemy::UpdateFloating(std::mt19937 randomEngine) {


	
	
	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(focusTargetPos_ - transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	// 回転補間
	transform_.rotate = Easing::Slerp(transform_.rotate, targetRotation, followSpeed_);

	// 移動方向の計算
	// ターゲットの周囲を回るための目標座標を計算
	orbitAngle_ += orbitSpeed_ * deltaTime_;  // 時間で角度加算
	if (orbitAngle_ > 2 * std::numbers::pi_v<float>) orbitAngle_ -= 2 * std::numbers::pi_v<float>;
	// ターゲットの周囲を回る座標を計算（XZ平面で円運動）
	Vector3 orbitPos;
	orbitPos.x = focusTargetPos_.x + orbitRadius_ * cos(orbitAngle_);
	orbitPos.y = focusTargetPos_.y; // 高さはターゲットに合わせる（必要に応じて調整）
	orbitPos.z = focusTargetPos_.z + orbitRadius_ * sin(orbitAngle_);
	// 目的座標までの方向ベクトルを計算
	toOrbitPos_ = orbitPos - transform_.translate;

	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
		moveDirection_ = Vector3Math::Normalize(toOrbitPos_);
		//移動時の加速度の計算
		velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
		velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;
	} else {
		velocity_.x /= deceleration_;
		velocity_.z /= deceleration_;
	}

	//最大移動速度の制限
	float speed = sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speed > kMaxMoveSpeed_) {
		float scale = kMaxMoveSpeed_ / speed;
		velocity_.x *= scale;
		velocity_.z *= scale;
	}

	// 空中での降下処理(fallSpeedを重力に加算)
	velocity_.y -= (gravity_ + fallSpeed_) * deltaTime_;
	transform_.translate.x += velocity_.x * deltaTime_;
	transform_.translate.z += velocity_.z * deltaTime_;
	transform_.translate.y += velocity_.y * deltaTime_;

	// 着地判定
	if (transform_.translate.y <= 0.0f) {
		transform_.translate.y = 0.0f;
		behaviorRequest_ = Behavior::RUNNING;
	}

	// --------------------------------
	// TODO: Enemyの浮遊時の挙動を実装する
	// --------------------------------

	// 浮遊中、LTボタンが押された場合STEPBOOSTに切り替え
	if (randomEngine() % 100 < 1.0f) {
		behaviorRequest_ = Behavior::STEPBOOST;
	}

	//浮遊中、再上昇する場合
	if(randomEngine() % 100 < 1.0f) {
		if( isOverheated_) {
			// オーバーヒート中はジャンプできない
			return;
		}
		// ジャンプのエネルギー消費
		energy_ -= useEnergyJump_ * deltaTime_;
		// ジャンプの速度を設定
		velocity_.y = jumpSpeed_;
	}
}

//===================================================================================
// 死亡時の処理
//===================================================================================

void Enemy::InitDead() {
	isAlive_ = false; // 死亡フラグを立てる
}

void Enemy::UpdateDead() {

	//空中にいる場合は落下処理
	if (transform_.translate.y > 0.0f) {
		// 空中での降下処理(fallSpeedを重力に加算)
		velocity_.y -= (gravity_ + fallSpeed_) * deltaTime_;
	} else {
		// 地面に着地したら速度を0にする
		velocity_.y = 0.0f;
	}

	//速度の減速処理
	velocity_.x /= deceleration_;
	velocity_.z /= deceleration_;

	//位置の更新（deltaTimeをここで適用）
	transform_.translate += velocity_ * deltaTime_;

}

//===================================================================================
// 攻撃開始判定
//===================================================================================

bool Enemy::ShouldStartAttack(int weaponIndex) {
	// 例: ターゲットとの距離が射程範囲でクールタイムが終わってたら攻撃
	auto* weapon = weapons_[weaponIndex].get();
	float distance = (focusTargetPos_ - transform_.translate).Length();
	float range = orbitRadius_ * 3.5f;
	bool cooldownReady = weapon->GetIsAvailable();
	// 例: 一定確率で攻撃開始
	return (distance <= range) && cooldownReady && (rand() % 100 < attackProbability_); // 10%の確率
}

bool Enemy::ShouldReleaseAttack(int weaponIndex) {
	auto* weapon = weapons_[weaponIndex].get();
	return weapon->GetChargeTime() >= weapon->GetRequiredChargeTime();
}

//===================================================================================
// エネルギーの更新処理
//===================================================================================

void Enemy::UpdateEnergy() {

	//オーバーヒートしているかどうか
	if(!isOverheated_) {

		// エネルギーの回復
		if (energy_ < maxEnergy_) {

			//浮遊状態,ジャンプ時、ステップブースト時はエネルギーを回復しない
			if (behavior_ == Behavior::FLOATING || behavior_ == Behavior::JUMP ||
				behavior_ == Behavior::STEPBOOST) {
				return;
			}

			energy_ += energyRegenRate_ * deltaTime_;
			if (energy_ > maxEnergy_) {
				energy_ = maxEnergy_;
			}
		}

		if(energy_ <= 0.0f) {
			// エネルギーが0以下になったらオーバーヒート状態にする
			isOverheated_ = true;
			overheatTimer_ = overheatDuration_; // オーバーヒートのタイマーを設定
			energy_ = 0.0f; // エネルギーを0にする
		}

	} else {

		// オーバーヒートの処理
		if (overheatTimer_ > 0.0f) {
			overheatTimer_ -= deltaTime_;
			if (overheatTimer_ <= 0.0f) {
				// オーバーヒートが終了したらエネルギーを半分回復
				energy_ = maxEnergy_ / 2.0f;
				// オーバーヒート状態を解除
				isOverheated_ = false;
			}
		}

	}
}