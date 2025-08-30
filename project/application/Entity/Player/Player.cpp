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

	// 地面に着地したらRUNNINGに戻る
	if( characterInfo_.onGround == true && 
		(behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP || 
		 behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING)) {
		behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
	}


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
	ImGui::Checkbox("OnGround", &characterInfo_.onGround);
	behaviorManager_->UpdateImGui();
	collider_->UpdateImGui("Player");
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
	if(other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//playerのColliderが接触している面で判断する
		if(collider_->GetSurfaceType() == SurfaceType::BOTTOM) {
			//playerが下面で接触している場合は地面にいると判定
			characterInfo_.onGround = true;
			characterInfo_.velocity.y = 0.0f;             // ジャンプ中の速度をリセット
			collider_->SetColor({ 0.0f,1.0f,0.0f,1.0f }); // コライダーの色を緑に変更

		} else if(collider_->GetSurfaceType() == SurfaceType::WALL) {
			//側面で接触した場合
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				characterInfo_.transform.translate += -normal * penetrationDepth;

				float dot = Vector3Math::Dot(characterInfo_.velocity, normal);
				if (dot < 0.0f) {
					characterInfo_.velocity -= normal * dot; // 法線方向の成分を除去
				}
			}
		
			collider_->SetColor({ 0.0f,0.0f,1.0f,1.0f }); // コライダーの色を青に変更

		} else if(collider_->GetSurfaceType() == SurfaceType::TOP) {
			//天井に接触した場合
			if(characterInfo_.velocity.y > 0.0f) {
				characterInfo_.velocity.y = 0.0f;
			}

			collider_->SetColor({ 1.0f,1.0f,0.0f,1.0f }); // コライダーの色を黄色に変更
		}
	}
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