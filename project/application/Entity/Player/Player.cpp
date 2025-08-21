#include "Player.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Model.h"
#include "engine/io/Input.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Easing.h"

#include "application/Weapon/Rifle.h"
#include "application/Weapon/Bazooka.h"
#include "application/Weapon/VerticalMissileLauncher.h"
#include "application/Entity/WeaponUnit.h"

#include "application/Entity/Behavior/BehaviorRunning.h"
#include "application/Entity/Behavior/BehaviorJumping.h"
#include "application/Entity/Behavior/BehaviorFloating.h"


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

	characterInfo_.transform = { {1.5f,1.5f,1.5f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,-30.0f} };
	object3d_->SetScale(characterInfo_.transform.scale);

	weapons_.resize(3);
	weaponTypes_.resize(3);
	weaponTypes_[R_ARMS] = WeaponType::WEAPON_TYPE_RIFLE; // 1つ目の武器はライフル
	weaponTypes_[L_ARMS] = WeaponType::WEAPON_TYPE_BAZOOKA; // 2つ目の武器はバズーカ
	weaponTypes_[R_BACK] = WeaponType::WEAPON_TYPE_VERTICAL_MISSILE; // 3つ目の武器は垂直ミサイル

	//背部エミッターの初期化
	backEmitter_ = std::make_unique<ParticleEmitter>();
	backEmitter_->Initialize("PalyerBackpack",object3d_->GetTransform(),10,0.01f);
	backEmitter_->SetParticleName("WalkSmoke2");
	gravity_ = 9.8f; // 重力の強さ

	inputProvider_ = std::make_unique<PlayerInputProvider>(this);

#pragma region charcterInfo

	characterInfo_.deceleration = 1.1f; //減速率
	characterInfo_.moveSpeed = 200.0f; //移動速度
	characterInfo_.kMaxMoveSpeed = 120.0f; //移動速度の最大値
	characterInfo_.maxHealth = 10000.0f; // 最大体力
	characterInfo_.health = characterInfo_.maxHealth; // 初期体力
	characterInfo_.stepBoostInfo.speed = 230.0f; // ステップブーストの速度
	characterInfo_.stepBoostInfo.duration = 0.2f; // ステップブーストの持続時間
	characterInfo_.stepBoostInfo.useEnergy = 100.0f; // ステップブーストに必要なエネルギー
	characterInfo_.stepBoostInfo.interval = 0.2f; // ステップブーストのインターバル
	characterInfo_.jumpInfo.speed = 50.0f; // ジャンプの速度
	characterInfo_.jumpInfo.maxJumpTime = 0.5f; // ジャンプの最大時間
	characterInfo_.jumpInfo.deceleration = 40.0f; // ジャンプ中の減速率
	characterInfo_.jumpInfo.useEnergy = 150.0f; // ジャンプに必要なエネルギー
	characterInfo_.chargeAttackStunInfo.stunDuration = 0.5f; // チャージ攻撃後の硬直時間
	characterInfo_.energyInfo.maxEnergy = 1000.0f; // 最大エネルギー
	characterInfo_.energyInfo.energy = characterInfo_.energyInfo.maxEnergy; // 初期エネルギー
	characterInfo_.energyInfo.recoveryRate = 200.0f; // エネルギーの回復速度
	characterInfo_.overHeatInfo.isOverheated = false; // オーバーヒート状態
	characterInfo_.overHeatInfo.overheatTimer = 0.0f; // オーバーヒートタイマー
	characterInfo_.overHeatInfo.overheatDuration = 3.0f; // オーバーヒートの持続時間

#pragma endregion

	//BehaviorManagerの初期化
	behaviorManager_ = std::make_unique<BehaviorManager>();
	behaviorManager_->Initialize(inputProvider_.get());
	behaviorManager_->InitializeBehaviors(characterInfo_);
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

	weapons_[R_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "RightHand"); // 1つ目の武器を右手に取り付け
	weapons_[R_ARMS]->SetUnitPosition(R_ARMS); // 1つ目の武器のユニットポジションを設定
	weapons_[L_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "LeftHand"); // 2つ目の武器を左手に取り付け
	weapons_[L_ARMS]->SetUnitPosition(L_ARMS); // 2つ目の武器のユニットポジションを設定
	weapons_[R_BACK]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "backpack.Left.Tip"); // 3つ目の武器を背中に取り付け
	weapons_[R_BACK]->SetUnitPosition(R_BACK); // 3つ目の武器のユニットポジションを設定
	weapons_[R_BACK]->SetRotate({ -1.0f, 0.0f, -1.5f }); // 背中の武器の回転を初期化
}

BaseWeapon* Player::GetWeapon(int index) const {
	return weapons_[index].get();
}

std::vector<std::unique_ptr<BaseWeapon>>& Player::GetWeapons() {
	return weapons_;
}

//===================================================================================
// 全体の更新処理
//===================================================================================

void Player::Update() {

	//stepBoostのインターバルの更新
	characterInfo_.stepBoostInfo.interval = 0.2f; // ステップブーストのインターバル
	if(characterInfo_.stepBoostInfo.intervalTimer > 0.0f) {
		characterInfo_.stepBoostInfo.intervalTimer -= deltaTime_;
	}

	// StepBoost入力判定を最初に追加
	if (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING ||
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING) {
		
		//オーバーヒート状態のチェック
		if (characterInfo_.overHeatInfo.isOverheated == false) {
			//StepBoost入力判定
			// LTボタン＋スティック入力で発動
			if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::LT)) {
				behaviorManager_->RequestBehavior(GameCharacterBehavior::STEPBOOST);
				//TriggerStepBoost();
			}
			//Jump入力判定
			//RTで発動
			if(characterInfo_.onGround == true){
			
				if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::RT)) {
					//ジャンプのリクエスト
					behaviorManager_->RequestBehavior(GameCharacterBehavior::JUMP);
					characterInfo_.onGround = false; // ジャンプしたので地上ではない
				}
			}
		}
	}

	//エネルギーの更新
	UpdateEnergy();

	characterInfo_.moveDirection = inputProvider_->GetMoveDirection();
	behaviorManager_->Update(characterInfo_);


	//攻撃処理
	if (characterInfo_.isAlive == true) {
		UpdateAttack();
	}

	if(characterInfo_.health <= 0.0f) {
		//死亡状態のリクエスト
		characterInfo_.isAlive = false;
		behaviorManager_->RequestBehavior(GameCharacterBehavior::DEAD);
	}

	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::toEuler(characterInfo_.transform.rotate);
	//カメラの設定
	camera_->SetFollowTargetPos(*object3d_->GetModel()->GetSkeleton()->GetJointPosition("neck",object3d_->GetWorldMatrix()));
	camera_->SetFollowTargetRot(eulerRotate);
	camera_->SetFocusTargetPos(focusTargetPos_);

	object3d_->SetTranslate(characterInfo_.transform.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->SetScale(characterInfo_.transform.scale);
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
	ImGui::DragFloat3("Translate", &characterInfo_.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &characterInfo_.transform.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &characterInfo_.transform.rotate.x, 0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &characterInfo_.health, 1.0f, 0.0f, characterInfo_.maxHealth);
	ImGui::DragFloat("Energy", &characterInfo_.energyInfo.energy, 1.0f, 0.0f, characterInfo_.energyInfo.maxEnergy);
	ImGui::DragFloat3("Velocity", &characterInfo_.velocity.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &characterInfo_.moveDirection.x, 0.01f);
	behaviorManager_->UpdateImGui();
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
	if (other->GetCharacterType() == CharacterType::ENEMY_BULLET) {
		//敵の弾に当たった場合
		collider_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
		//ダメージを受ける
		Bullet* bullet = dynamic_cast<Bullet*>(other);
		characterInfo_.health -= bullet->GetDamage();
	}
	if(other->GetCharacterType() == CharacterType::ENEMY_MISSILE) {
		//敵のミサイルに当たった場合
		collider_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
		//ダメージを受ける
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		characterInfo_.health -= missile->GetDamage();
	}
}

//===================================================================================
//　移動処理
//===================================================================================



void Player::InitRunning() {}

void Player::UpdateRunning() {

	Vector3& moveDirection = characterInfo_.moveDirection; // 移動方向
	Vector3& velocity = characterInfo_.velocity; // 移動ベクトル
	float moveSpeed = characterInfo_.moveSpeed; // 移動速度
	float deceleration = characterInfo_.deceleration; // 減速率

	//左スティック
	StickState leftStick= Input::GetInstance()->GetLeftStickState(0);
	//右スティック 
	StickState rightStick = Input::GetInstance()->GetRightStickState(0);

	//カメラの回転を設定
	camera_->SetStick({ rightStick.x, rightStick.y });

	//カメラの方向から移動方向の計算
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	moveDirection = forward * leftStick.y + right * leftStick.x;

	//移動方向の正規化
	if (moveDirection.x != 0.0f || moveDirection.z != 0.0f) {
		moveDirection = Vector3Math::Normalize(moveDirection);
		//移動時の加速度の計算
		velocity.x += moveDirection.x * moveSpeed * deltaTime_;
		velocity.z += moveDirection.z * moveSpeed * deltaTime_;
		
		float targetAngle = atan2(moveDirection.x, moveDirection.z);
		Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, targetAngle);
		
		characterInfo_.transform.rotate = Easing::Slerp(characterInfo_.transform.rotate, targetRotate, 0.1f);
		characterInfo_.transform.rotate = QuaternionMath::Normalize(characterInfo_.transform.rotate);
		//パーティクルエミッターの更新
		backEmitter_->Emit();

	} else {
		//速度の減速処理
		characterInfo_.velocity.x /= deceleration;
		characterInfo_.velocity.z /= deceleration;
	}

	//最大移動速度の制限
	float speed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
	if (speed > characterInfo_.kMaxMoveSpeed) {
		float scale = characterInfo_.kMaxMoveSpeed / speed;
		velocity.x *= scale;
		velocity.z *= scale;
	}

	//移動処理
	// 位置の更新（deltaTimeをここで適用）
	characterInfo_.transform.translate.x += velocity.x * deltaTime_;
	characterInfo_.transform.translate.z += velocity.z * deltaTime_;
}

//===================================================================================
//　攻撃処理
//===================================================================================

void Player::UpdateAttack() {

	WeaponAttack(R_ARMS, GamepadButtonType::RB); // 1つ目の武器の攻撃
	WeaponAttack(L_ARMS, GamepadButtonType::LB); // 2つ目の武器の攻撃
	WeaponAttack(R_BACK, GamepadButtonType::X); // 3つ目の武器の攻撃
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
					behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
				} else {
					// 移動撃ち可能な場合はRUNNINGに戻す
					behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
				}
			} 
		} else {
			//チャージ攻撃不可:通常攻撃
			if (weapon->IsStopShootOnly() && weapon->GetAttackInterval() <= 0.0f) {
				// 停止撃ち専用:硬直処理を行う
				characterInfo_.isChargeShooting = true; // チャージ撃ち中フラグを立てる
				chargeShootTimer_ = chargeShootDuration_; // チャージ撃ちのタイマーを設定

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
				behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
			} else {
				// 移動撃ち可能な場合はRUNNINGに戻す
				behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
			}
		}
	}

	if(characterInfo_.isChargeShooting == true) {
		// チャージ撃ち中の処理
		chargeShootTimer_ -= deltaTime_;
		if (chargeShootTimer_ <= 0.0f) {
			weapon->Attack();
			characterInfo_.isChargeShooting = false; // チャージ撃ち中フラグをリセット
			chargeShootTimer_ = 0.0f; // タイマーをリセット
			behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
		}
	}
}

void Player::WeaponChargeAttack(int weaponIndex) {

	Vector3& velocity_ = characterInfo_.velocity; // 移動ベクトル

	// 速度を大きく落としていき、停止させた後攻撃処理を入れる
	velocity_.x *= 0.5f;
	velocity_.z *= 0.5f;
	if (std::abs(velocity_.x) < 0.01f && std::abs(velocity_.z) < 0.01f) {
		// チャージ攻撃の実行
		auto* weapon = weapons_[weaponIndex].get();
			if (weapon->IsChargeAttack()) {
				weapon->ChargeAttack();

			}else {
				// チャージ攻撃不可の場合は通常攻撃
				weapon->Attack();
			}

			if (weapon->IsMoveShootable()) {
				// ステップ終了時前回の状態に戻す
				if (characterInfo_.transform.translate.y <= 0.0f) {
					behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
				} else if (characterInfo_.transform.translate.y > 0.0f) {
					behaviorManager_->RequestBehavior(GameCharacterBehavior::FLOATING);
				} else {
					behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
				}
			} else {
				// 停止撃ち専用の場合はCHARGESHOOT_STUNに切り替え
				behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
			}
		
	}

	// 位置の更新（deltaTimeをここで適用）
	characterInfo_.transform.translate.x += velocity_.x * deltaTime_;
	characterInfo_.transform.translate.z += velocity_.z * deltaTime_;
}

//===================================================================================
//　ジャンプ処理
//===================================================================================

void Player::InitJump() {

	//オーバーヒート状態のチェック
	if (characterInfo_.overHeatInfo.isOverheated) {
		// オーバーヒート中はジャンプできない
		//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		return;
	}

	// ジャンプのエネルギー消費
	characterInfo_.energyInfo.energy -= characterInfo_.jumpInfo.useEnergy;

	//ジャンプの初期化
	//ジャンプの速度を設定
	characterInfo_.velocity.y = characterInfo_.jumpInfo.speed;
	//ジャンプ中の移動方向を設定
	StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
	characterInfo_.moveDirection = forward * leftStick.y + right * leftStick.x;
	
}

void Player::UpdateJump() {

	Vector3& velocity = characterInfo_.velocity; // 移動ベクトル

	// ジャンプ中の移動
	characterInfo_.transform.translate.x += velocity.x * deltaTime_;
	characterInfo_.transform.translate.z += velocity.z * deltaTime_;
	// 重力の適用
	velocity.y -= (gravity_ + characterInfo_.jumpInfo.deceleration) * deltaTime_;
	characterInfo_.transform.translate.y += velocity.y * deltaTime_;

	characterInfo_.jumpInfo.jumpTimer += deltaTime_;
	 if (characterInfo_.jumpInfo.jumpTimer > characterInfo_.jumpInfo.maxJumpTime) {
	     //behaviorRequest_ = GameCharacterBehavior::FLOATING;
	     return;
	 }

	// 地面に着地したらRUNNINGに戻る
	if (characterInfo_.transform.translate.y <= 0.0f) {
		characterInfo_.transform.translate.y = 0.0f; // 地面に合わせる
		//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		velocity = { 0.0f, 0.0f, 0.0f }; // ジャンプ中の速度をリセット
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

	Vector3& velocity_ = characterInfo_.velocity; // 移動ベクトル

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
				if (characterInfo_.transform.translate.y <= 0.0f) {
					//behaviorRequest_ = GameCharacterBehavior::RUNNING;
				} else if (characterInfo_.transform.translate.y > 0.0f) {
					//behaviorRequest_ = GameCharacterBehavior::FLOATING;
				} else {
					// ステップブーストが終了したらRUNNINGに戻す
					//behaviorRequest_ = GameCharacterBehavior::RUNNING;
				}
			} else {
				// 停止撃ち専用の場合はCHARGESHOOT_STUNに切り替え
				//behaviorRequest_ = GameCharacterBehavior::CHARGESHOOT_STUN;
			}
		}
	}

	// 位置の更新（deltaTimeをここで適用）
	characterInfo_.transform.translate.x += velocity_.x * deltaTime_;
	characterInfo_.transform.translate.z += velocity_.z * deltaTime_;
}

//===================================================================================
//　チャージ攻撃後の硬直処理
//===================================================================================

void Player::InitChargeShootStun() {
	characterInfo_.chargeAttackStunInfo.stunTimer = characterInfo_.chargeAttackStunInfo.stunDuration;
}

void Player::UpdateChargeShootStun() {

	float& stunTimer_ = characterInfo_.chargeAttackStunInfo.stunTimer;

	stunTimer_ -= deltaTime_;
	if (stunTimer_ <= 0.0f) {
		// ステップ終了時前回の状態に戻す
		if (characterInfo_.transform.translate.y <= 0.0f) {
			//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		} else if (characterInfo_.transform.translate.y > 0.0f) {
			//behaviorRequest_ = GameCharacterBehavior::FLOATING;
		} else {
			// ステップブーストが終了したらRUNNINGに戻す
			//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		}
	}
}

//===================================================================================
//　ステップブースト処理
//===================================================================================


void Player::InitStepBoost() {

	float speed = characterInfo_.stepBoostInfo.speed;
	float duration = characterInfo_.stepBoostInfo.duration;
	float useEnergy = characterInfo_.stepBoostInfo.useEnergy;
	
	Vector3 direction = characterInfo_.stepBoostInfo.direction;

	// オーバーヒート状態のチェック
	if (characterInfo_.overHeatInfo.isOverheated) {
		// オーバーヒート中はステップブーストできない
		//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		return;
	}

	// ステップブーストのエネルギーを消費
	characterInfo_.energyInfo.energy -= useEnergy;

	//上昇速度を急激に遅くする
	characterInfo_.velocity.y = 0.0f;

	characterInfo_.stepBoostInfo.boostTimer = duration;
	characterInfo_.velocity.x = direction.x * speed;
	characterInfo_.velocity.z = direction.z * speed;
}


void Player::UpdateStepBoost() {

	float interval = characterInfo_.stepBoostInfo.interval;

	// ステップ中の移動
	characterInfo_.transform.translate.x += characterInfo_.velocity.x * deltaTime_;
	characterInfo_.transform.translate.z += characterInfo_.velocity.z * deltaTime_;

	characterInfo_.stepBoostInfo.boostTimer -= deltaTime_;
	if (characterInfo_.stepBoostInfo.boostTimer <= 0.0f) {
		// ステップ終了時前回の状態に戻す
		if (characterInfo_.transform.translate.y <= 0.0f) {
			//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		} else if (characterInfo_.transform.translate.y > 0.0f) {
			//behaviorRequest_ = GameCharacterBehavior::FLOATING;
		} else {
			// ステップブーストが終了したらRUNNINGに戻す
			//behaviorRequest_ = GameCharacterBehavior::RUNNING;
		}

		// ステップブーストのインターバルをリセット
		characterInfo_.stepBoostInfo.intervalTimer = interval;
	}
}

void Player::TriggerStepBoost() {

	Vector3& direction = characterInfo_.stepBoostInfo.direction;

	if (characterInfo_.stepBoostInfo.intervalTimer <= 0.0f) {
		StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
		if (fabs(leftStick.x) > 0.2f || fabs(leftStick.y) > 0.2f) {
			//方向ベクトル計算（カメラ考慮）
			Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
			Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
			direction = forward * leftStick.y + right * leftStick.x;
			direction = Vector3Math::Normalize(direction);
			//behaviorRequest_ = GameCharacterBehavior::STEPBOOST;
		}
	}
}
//
//
//
////===================================================================================
//// 浮遊時の処理
////===================================================================================
//
//void Player::InitFloating() {
//	
//}
//
//void Player::UpdateFloating() {
//
//	Vector3& moveDirection_ = characterInfo_.moveDirection; // 移動方向
//	Vector3& velocity_ = characterInfo_.velocity; // 移動ベクトル
//	float moveSpeed_ = characterInfo_.moveSpeed; // 移動速度
//	float deceleration_ = characterInfo_.deceleration; // 減速率
//	float kMaxMoveSpeed_ = characterInfo_.kMaxMoveSpeed; // 最大移動速度
//
//	// スティックで水平方向に自由に動かす
//	StickState leftStick = Input::GetInstance()->GetLeftStickState(0);
//	StickState rightStick = Input::GetInstance()->GetRightStickState(0);
//
//	camera_->SetStick({ rightStick.x, rightStick.y });
//
//	Vector3 forward = QuaternionMath::RotateVector(Vector3(0.0f, 0.0f, 1.0f), camera_->GetRotate());
//	Vector3 right = QuaternionMath::RotateVector(Vector3(1, 0, 0), camera_->GetRotate());
//	moveDirection_ = forward * leftStick.y + right * leftStick.x;
//
//	
//
//	if (moveDirection_.x != 0.0f || moveDirection_.z != 0.0f) {
//		moveDirection_ = Vector3Math::Normalize(moveDirection_);
//		//移動時の加速度の計算
//		velocity_.x += moveDirection_.x * moveSpeed_ * deltaTime_;
//		velocity_.z += moveDirection_.z * moveSpeed_ * deltaTime_;
//	} else {
//		velocity_.x /= deceleration_;
//		velocity_.z /= deceleration_;
//	}
//
//	//最大移動速度の制限
//	float speed = sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
//	if (speed > kMaxMoveSpeed_) {
//		float scale = kMaxMoveSpeed_ / speed;
//		velocity_.x *= scale;
//		velocity_.z *= scale;
//	}
//
//	// 空中での降下処理(fallSpeedを重力に加算)
//	velocity_.y -= (gravity_ + characterInfo_.fallSpeed) * deltaTime_;
//	characterInfo_.transform.translate.x += velocity_.x * deltaTime_;
//	characterInfo_.transform.translate.z += velocity_.z * deltaTime_;
//	characterInfo_.transform.translate.y += velocity_.y * deltaTime_;
//
//	// 着地判定
//	if (characterInfo_.transform.translate.y <= 0.0f) {
//		characterInfo_.transform.translate.y = 0.0f;
//		behaviorRequest_ = GameCharacterBehavior::RUNNING;
//	}
//
//	// 浮遊中、LTボタンが押された場合STEPBOOSTに切り替え
//	// LTボタン＋スティック入力で発動
//	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::LT)) {
//		TriggerStepBoost();
//	}
//	//ジャンプボタンの追加入力でさらに上昇
//	if( Input::GetInstance()->PushButton(0, GamepadButtonType::RT)) {
//
//		if( characterInfo_.overHeatInfo.isOverheated) {
//			// オーバーヒート中はジャンプできない
//			return;
//		}
//
//		// ジャンプのエネルギー消費
//		characterInfo_.energyInfo.energy -= characterInfo_.jumpInfo.useEnergy * deltaTime_;
//		// ジャンプの速度を設定
//		velocity_.y = characterInfo_.jumpInfo.speed;
//	}
//}
//
////===================================================================================
//// 死亡時の処理
////===================================================================================
//
//void Player::InitDead() {
//	characterInfo_.isAlive = false; // 死亡フラグを立てる
//}
//
//void Player::UpdateDead() {
//
//	Vector3& velocity_ = characterInfo_.velocity; // 移動ベクトル
//	float deceleration_ = characterInfo_.deceleration; // 減速率
//	float fallSpeed_ = characterInfo_.fallSpeed; // 落下速度
//
//	//左スティック
//	StickState leftStick= Input::GetInstance()->GetLeftStickState(0);
//	//右スティック 
//	StickState rightStick = Input::GetInstance()->GetRightStickState(0);
//
//	//カメラの回転を設定
//	camera_->SetStick({ rightStick.x, rightStick.y });
//
//	//空中にいる場合は落下処理
//	if (characterInfo_.transform.translate.y > 0.0f) {
//		// 空中での降下処理(fallSpeedを重力に加算)
//		velocity_.y -= (gravity_ + fallSpeed_) * deltaTime_;
//	} else {
//		// 地面に着地したら速度を0にする
//		velocity_.y = 0.0f;
//	}
//
//	//速度の減速処理
//	velocity_.x /= deceleration_;
//	velocity_.z /= deceleration_;
//
//	//位置の更新（deltaTimeをここで適用）
//	characterInfo_.transform.translate += velocity_ * deltaTime_;
//
//}
//
//===================================================================================
// エネルギーの更新処理
//===================================================================================

void Player::UpdateEnergy() {

	float& overheatTimer = characterInfo_.overHeatInfo.overheatTimer; // オーバーヒートタイマー
	float maxEnergy = characterInfo_.energyInfo.maxEnergy; // 最大エネルギー

	//オーバーヒートしているかどうか
	if(!characterInfo_.overHeatInfo.isOverheated) {

		if(characterInfo_.energyInfo.energy <= 0.0f) {
			// エネルギーが0以下になったらオーバーヒート状態にする
			characterInfo_.overHeatInfo.isOverheated = true;
			overheatTimer = characterInfo_.overHeatInfo.overheatDuration; // オーバーヒートのタイマーを設定
			characterInfo_.energyInfo.energy = 0.0f; // エネルギーを0にする
		}

		// エネルギーの回復
		if (characterInfo_.energyInfo.energy < maxEnergy) {

			//浮遊状態,ジャンプ時、ステップブースト時はエネルギーを回復しない
			if (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING ||
				behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP ||
				behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::STEPBOOST) {
				return; // エネルギーの回復を行わない
			}

			characterInfo_.energyInfo.energy += characterInfo_.energyInfo.recoveryRate * deltaTime_;
			if (characterInfo_.energyInfo.energy > maxEnergy) {
				characterInfo_.energyInfo.energy = maxEnergy;
			}
		}
		
	} else {

		// オーバーヒートの処理
		if (overheatTimer > 0.0f) {
			overheatTimer-= deltaTime_;
			if (overheatTimer <= 0.0f) {
				// オーバーヒートが終了したらエネルギーを半分回復
				characterInfo_.energyInfo.energy = maxEnergy / 2.0f;
				// オーバーヒート状態を解除
				characterInfo_.overHeatInfo.isOverheated = false;
			}
		}

	}
}

//void Player::ChangeBehavior(std::unique_ptr<BaseBehavior> behavior) {
//	behaviorPtr_ = std::move(behavior);
//}