#include "Player.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Model.h"
#include "Input.h"
#include "camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "Vector3Math.h"
#include "math/Easing.h"

#include "Weapon/Rifle.h"
#include "Weapon/Bazooka.h"
#include "Weapon/VerticalMissileLauncher.h"

Player::~Player() {
	object3d_.reset();
	collider_.reset();
	camera_ = nullptr;
}

//===================================================================================
// 初期化処理
//===================================================================================

void Player::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//キャラクタータイプ設定
	characterType_ = CharacterType::PLAYER;

	behaviorRequest_ = Behavior::RUNNING;
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetHalfSize({ 2.0f, 2.5f, 2.0f }); // コライダーの半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Player));

	camera_ = object3dCommon->GetDefaultCamera();
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	transform_ = { {1.5f,1.5f,1.5f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,-30.0f} };
	object3d_->SetScale(transform_.scale);

	weapons_.resize(3);
	weaponTypes_.resize(3);
	weaponTypes_[0] = WeaponType::WEAPON_TYPE_RIFLE; // 1つ目の武器はライフル
	weaponTypes_[1] = WeaponType::WEAPON_TYPE_BAZOOKA; // 2つ目の武器はバズーカ
	weaponTypes_[2] = WeaponType::WEAPON_TYPE_VERTICAL_MISSILE; // 3つ目の武器は垂直ミサイル

	//背部エミッターの初期化
	backEmitter_ = std::make_unique<ParticleEmitter>();
	backEmitter_->Initialize("PalyerBackpack",object3d_->GetTransform(),10,0.01f);
	backEmitter_->SetParticleName("WalkSmoke2");

	//体力の初期化
	health_ = 10000.0f;
}

//===================================================================================
// 武器の初期化処理
//===================================================================================

void Player::WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager) {
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
		}else if(weaponTypes_[i] == WeaponType::WEAPON_TYPE_VERTICAL_MISSILE) {
			//垂直ミサイルの武器を初期化
			weapons_[i] = std::make_unique<VerticalMissileLauncher>();
			weapons_[i]->Initialize(object3dCommon,bulletManager, "Bazooka.gltf");
			weapons_[i]->SetOwnerObject(this);
		} else {
			weapons_[i] = nullptr; // 未使用の武器スロットはnullptrに設定
		}
	}

	weapons_[0]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "RightHand"); // 1つ目の武器を右手に取り付け
	weapons_[1]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "LeftHand"); // 2つ目の武器を左手に取り付け
	weapons_[2]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "backpack"); // 3つ目の武器を背中に取り付け
}

//===================================================================================
// 全体の更新処理
//===================================================================================

void Player::Update() {

	//stepBoostのインターバルの更新
	if(stepBoostIntervalTimer_ > 0.0f) {
		stepBoostIntervalTimer_ -= deltaTime_;
	}

	// StepBoost入力判定を最初に追加
	if (behavior_ == Behavior::RUNNING) {
		
		//StepBoost入力判定
		// LTボタン＋スティック入力で発動
		if (Input::GetInstance()->PushButton(0, GamepadButtonType::LT)) {
			TriggerStepBoost();
		}
		//Jump入力判定
		//RTで発動
		if(Input::GetInstance()->TriggerButton(0, GamepadButtonType::RT)) {
			//ジャンプのリクエスト
			behaviorRequest_ = Behavior::JUMP;
		}
	}

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
		}

		behaviorRequest_ = std::nullopt;
	}

	switch (behavior_) {
	case Player::Behavior::IDLE:
		break;
	case Player::Behavior::RUNNING:
		UpdateRunning();
		break;
	case Player::Behavior::JUMP:
		UpdateJump();
		break;
	case Player::Behavior::DASH:
		UpdateDash();
		break;
	case Player::Behavior::STEPBOOST:
		UpdateStepBoost();
		break;
	case Player::Behavior::FLOATING:
		UpdateFloating();
		break;
	case Player::Behavior::CHARGESHOOT:
		UpdateChargeShoot();
		break;
	case Player::Behavior::CHARGESHOOT_STUN:
		UpdateChargeShootStun();
		break;
	default:
		break;
	}

	//攻撃処理
	UpdateAttack();

	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::toEuler(transform_.rotate);
	//カメラの設定
	camera_->SetFollowTargetPos(*object3d_->GetModel()->GetSkeleton()->GetJointPosition("neck",object3d_->GetWorldMatrix()));
	camera_->SetFollowTargetRot(eulerRotate);
	camera_->SetFocusTargetPos(focusTargetPos_);

	object3d_->SetTranslate(transform_.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	std::optional<Vector3> backpackPosition = object3d_->GetModel()->GetSkeleton()->GetJointPosition("leg",object3d_->GetWorldMatrix());
	backEmitter_->SetTranslate(backpackPosition.value());
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("WalkSmoke2")->SetEmitterPosition(backpackPosition.value());
	backEmitter_->Update();

	for(const auto& weapon : weapons_) {
		if (weapon) {
			weapon->SetTarget(focusTargetPos_);
			weapon->Update();
		}
	}
}

//===================================================================================
// ImGuiの更新処理
//===================================================================================

void Player::UpdateImGui() {
#ifdef _DEBUG

	ImGui::Begin("Player");
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &health_, 1.0f, 0.0f, maxHealth_);
	ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &moveDirection_.x, 0.01f);
	ImGui::Text("Behavior: %d", static_cast<int>(behavior_));
	weapons_[0]->UpdateImGui();
	weapons_[1]->UpdateImGui();
	weapons_[2]->UpdateImGui(); 
	ImGui::End();

#endif // _DEBUG
}

//===================================================================================
// 描画処理
//===================================================================================

void Player::Draw() {
	object3d_->Draw();
	for(const auto& weapon : weapons_) {
		if (weapon) {
			weapon->Draw();
		}
	}
}

void Player::DrawCollider() {
#ifdef _DEBUG
	collider_->DrawCollider();
	object3d_->GetModel()->GetSkeleton()->Draw(object3d_->GetWorldMatrix());
	backEmitter_->DrawWireFrame();
#endif
}

//===================================================================================
// 衝突時の処理
//===================================================================================

void Player::OnCollisionAction(GameCharacter* other) {

	if (other->GetCharacterType() == CharacterType::ENEMY) {
		//衝突時の処理
		collider_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
	}
}

//===================================================================================
//　移動処理
//===================================================================================

void Player::InitRunning() {}

void Player::UpdateRunning() {

	//左スティック
	StickState leftStick= Input::GetInstance()->GetLeftStickState(0);
	//右スティック 
	StickState rightStick = Input::GetInstance()->GetRightStickState(0);

	//カメラの回転を設定
	camera_->SetStick({ rightStick.x, rightStick.y });

	//カメラの方向から移動方向の計算
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	moveDirection_ = forward * leftStick.y + right * leftStick.x;

	//移動方向の正規化
	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
		moveDirection_ = Vector3Math::Normalize(moveDirection_);
		//移動時の加速度の計算
		velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
		velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;
		
		float targetAngle = atan2(moveDirection_.x, moveDirection_.z);
		Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, targetAngle);
		
		transform_.rotate = Easing::Slerp(transform_.rotate, targetRotate, 0.1f);
		transform_.rotate = QuaternionMath::Normalize(transform_.rotate);
		//パーティクルエミッターの更新
		backEmitter_->Emit();

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
//　攻撃処理
//===================================================================================

void Player::UpdateAttack() {

	WeaponAttack(0, GamepadButtonType::RB); // 1つ目の武器の攻撃
	WeaponAttack(1, GamepadButtonType::LB); // 2つ目の武器の攻撃
	WeaponAttack(2, GamepadButtonType::X); // 3つ目の武器の攻撃
}

void Player::WeaponAttack(int weaponIndex, GamepadButtonType buttonType) {

	auto* weapon = weapons_[weaponIndex].get();
	if (Input::GetInstance()->PushButton(0, buttonType)) {
		//チャージ攻撃可能な場合
		if (weapon->IsChargeAttack()) {

			//武器のチャージ処理
			weapon->Charge(deltaTime_);
			if(Input::GetInstance()->ReleaseButton(0, buttonType)) {
				//チャージ攻撃実行
				weapon->ChargeAttack();
				if (weapon->IsStopShootOnly()) {
					// 停止撃ち専用の場合はチャージ後に硬直状態へ
					behaviorRequest_ = Behavior::CHARGESHOOT_STUN;
				} else {
					// 移動撃ち可能な場合はRUNNINGに戻す
					behaviorRequest_ = Behavior::RUNNING;
				}
			} 
		} else {
			//チャージ攻撃不可:通常攻撃
			if (weapon->IsStopShootOnly() && weapon->GetAttackInterval() <= 0.0f) {
				// 停止撃ち専用:硬直処理を行う
				behaviorRequest_ = Behavior::CHARGESHOOT;
			} else {
				// 移動撃ち可能
				weapon->Attack();
			}
		}
	} else if (Input::GetInstance()->ReleaseButton(0, buttonType)) {
		// LBボタンが離された場合

		if (weapon->IsCharging()) {
			// チャージ中の場合はチャージ攻撃を終了
			weapon->ChargeAttack();
			if (weapon->IsStopShootOnly()) {
				// 停止撃ち専用の場合はチャージ後に硬直状態へ
				behaviorRequest_ = Behavior::CHARGESHOOT_STUN;
			} else {
				// 移動撃ち可能な場合はRUNNINGに戻す
				behaviorRequest_ = Behavior::RUNNING;
			}
		}
	}
}

//===================================================================================
//　ジャンプ処理
//===================================================================================

void Player::InitJump() {

	//ジャンプの初期化
	//ジャンプの速度を設定
	velocity_.y = jumpSpeed_;
	//ジャンプ中の移動方向を設定
	StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	moveDirection_ = forward * leftStick.y + right * leftStick.x;
	transform_.translate.y += 0.1f; // 少し上に移動してジャンプ感を出す
}

void Player::UpdateJump() {

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


void Player::InitDash() {

}


void Player::UpdateDash() {
}


//===================================================================================
//　チャージ攻撃処理
//===================================================================================
void Player::InitChargeShoot() {

}
void Player::UpdateChargeShoot() {

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

void Player::InitChargeShootStun() {
	chargeAttackStunTimer_ = chargeAttackStunDuration_;
}

void Player::UpdateChargeShootStun() {

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


void Player::InitStepBoost() {

	//上昇速度を急激に遅くする
	velocity_.y = 0.0f;

	stepBoostTimer_ = stepBoostDuration_;
	velocity_.x = stepBoostDirection_.x * stepBoostSpeed_;
	velocity_.z = stepBoostDirection_.z * stepBoostSpeed_;
}

void Player::UpdateStepBoost() {
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

void Player::TriggerStepBoost() {
	if (stepBoostIntervalTimer_ <= 0.0f) {
		StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
		if (fabs(leftStick.x) > 0.2f || fabs(leftStick.y) > 0.2f) {
			//方向ベクトル計算（カメラ考慮）
			Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
			Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
			stepBoostDirection_ = forward * leftStick.y + right * leftStick.x;
			stepBoostDirection_ = Vector3Math::Normalize(stepBoostDirection_);
			behaviorRequest_ = Behavior::STEPBOOST;
		}
	}
}



//===================================================================================
// 浮遊時の処理
//===================================================================================

void Player::InitFloating() {
	
}

void Player::UpdateFloating() {

	// 浮遊中、LTボタンが押された場合STEPBOOSTに切り替え
	// LTボタン＋スティック入力で発動
	if (Input::GetInstance()->PushButton(0, GamepadButtonType::LT)) {
		TriggerStepBoost();
	}
	// スティックで水平方向に自由に動かす
	StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
	StickState rightStick = Input::GetInstance()->GetRightStickState(0);

	camera_->SetStick({ rightStick.x, rightStick.y });

	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	moveDirection_ = forward * leftStick.y + right * leftStick.x;

	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
		moveDirection_ = Vector3Math::Normalize(moveDirection_);
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
}