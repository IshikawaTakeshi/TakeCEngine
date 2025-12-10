#include "Player.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Model.h"
#include "engine/Input/Input.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Easing.h"
#include "engine/Utility/StringUtility.h"
#include "engine/Utility/JsonLoader.h"

#include "application/Weapon/Rifle/Rifle.h"
#include "application/Weapon/Bazooka/Bazooka.h"
#include "application/Weapon/Launcher/VerticalMissileLauncher.h"
#include "application/Weapon/MachineGun/MachineGun.h"
#include "application/Weapon/ShotGun/ShotGun.h"
#include "application/Entity/WeaponUnit.h"

#include "application/Entity/Behavior/BehaviorRunning.h"
#include "application/Entity/Behavior/BehaviorJumping.h"
#include "application/Entity/Behavior/BehaviorFloating.h"
#include "application/Effect/BoostEffectPositionEnum.h"

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

	playerData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<CharacterData>("PlayerData.json");
	playerData_.characterInfo.transform = { {1.5f,1.5f,1.5f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,-30.0f} };
	object3d_->SetScale(playerData_.characterInfo.transform.scale);

	weapons_.resize(WeaponUnit::Size);
	weaponTypes_.resize(WeaponUnit::Size);
	for(size_t i = 0; i < weaponTypes_.size(); i++) {
		weaponTypes_[i] = playerData_.weaponData[i].weaponType;
	}
	chargeShootableUnits_.resize(4);

	//背部エミッターの初期化
	backEmitter_ = std::make_unique<ParticleEmitter>();
	backEmitter_->Initialize("PlayerBackpack", object3d_->GetTransform(), 10, 0.01f);
	backEmitter_->SetParticleName("WalkSmoke2");
	//backEmitter_->SetIsEmit(true);

	//ブーストエフェクトの初期化
	boostEffects_.resize(4);
	for (int i = 0; i < 4; i++) {
		boostEffects_[i] = std::make_unique<BoostEffect>();
		boostEffects_[i]->Initialize(this);
	}
	boostEffects_[LEFT_BACK]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "backpack.Left.Tip");
	boostEffects_[RIGHT_BACK]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "backpack.Right.Tip");
	boostEffects_[LEFT_SHOULDER]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "LeftShoulder");
	boostEffects_[RIGHT_SHOULDER]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "RightShoulder");
	boostEffects_[LEFT_BACK]->SetRotate({ 0.7f,0.0f,1.3f });
	boostEffects_[RIGHT_BACK]->SetRotate({ 0.7f,0.0f,-1.3f }); //エフェクトの向きを調整

	//入力プロバイダーの初期化
	inputProvider_ = std::make_unique<PlayerInputProvider>(this);

	//BehaviorManagerの初期化
	behaviorManager_ = std::make_unique<BehaviorManager>();
	behaviorManager_->Initialize(inputProvider_.get());
	behaviorManager_->InitializeBehaviors(playerData_.characterInfo);
}

//===================================================================================
// 武器の初期化処理
//===================================================================================

void Player::WeaponInitialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {
	//武器の初期化
	for (int i = 0; i < weapons_.size() - 1; i++) {
		if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_RIFLE) {
			weapons_[i] = std::make_unique<Rifle>();
			weapons_[i]->Initialize(object3dCommon, bulletManager);
			weapons_[i]->SetOwnerObject(this);
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_BAZOOKA) {
			weapons_[i] = std::make_unique<Bazooka>();
			weapons_[i]->Initialize(object3dCommon, bulletManager);
			weapons_[i]->SetOwnerObject(this);
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_VERTICAL_MISSILE) {
			//垂直ミサイルの武器を初期化
			weapons_[i] = std::make_unique<VerticalMissileLauncher>();
			weapons_[i]->Initialize(object3dCommon, bulletManager);
			weapons_[i]->SetOwnerObject(this);
		} else if(weaponTypes_[i] == WeaponType::WEAPON_TYPE_MACHINE_GUN) {
			//マシンガンの武器を初期化
			weapons_[i] = std::make_unique<MachineGun>();
			weapons_[i]->Initialize(object3dCommon, bulletManager);
			weapons_[i]->SetOwnerObject(this);
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_SHOTGUN) {
			//ショットガンの武器を初期化
			weapons_[i] = std::make_unique<ShotGun>();
			weapons_[i]->Initialize(object3dCommon, bulletManager);
			weapons_[i]->SetOwnerObject(this);
		} else {
			//武器が設定されていない場合はnullptrのまま
			weapons_[i] = nullptr;
		}
		weapons_[3] = std::make_unique<ShotGun>();
		weapons_[3]->Initialize(object3dCommon, bulletManager);
		weapons_[3]->SetOwnerObject(this);
	}

	weapons_[R_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "RightHand"); // 1つ目の武器を右手に取り付け
	weapons_[R_ARMS]->SetUnitPosition(R_ARMS); // 1つ目の武器のユニットポジションを設定
	weapons_[L_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "LeftHand"); // 2つ目の武器を左手に取り付け
	weapons_[L_ARMS]->SetUnitPosition(L_ARMS); // 2つ目の武器のユニットポジションを設定
	weapons_[R_BACK]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "backpack.Left.Tip"); // 3つ目の武器を背中に取り付け
	weapons_[R_BACK]->SetUnitPosition(R_BACK); // 3つ目の武器のユニットポジションを設定
	weapons_[R_BACK]->SetRotate({ -1.0f, 0.0f, -2.0f }); // 背中の武器の回転を初期化
	weapons_[L_BACK]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "backpack.Right.Tip"); // 4つ目の武器を背中に取り付け
	weapons_[L_BACK]->SetUnitPosition(L_BACK); // 4つ目の武器のユニットポジションを設定
	weapons_[L_BACK]->SetRotate({ -1.0f, 0.0f, 2.0f }); // 背中の武器の回転を初期化
}

//===================================================================================
// 武器の取得
//===================================================================================
BaseWeapon* Player::GetCurrentWeapon(int index) const {
	return weapons_[index].get();
}

//===================================================================================
// 全武器の取得
//===================================================================================
std::vector<std::unique_ptr<BaseWeapon>>& Player::GetWeapons() {
	return weapons_;
}

//===================================================================================
// 全体の更新処理
//===================================================================================

void Player::Update() {

	//stepBoostのインターバルの更新
	playerData_.characterInfo.stepBoostInfo.interval = 0.2f; // ステップブーストのインターバル
	if (playerData_.characterInfo.stepBoostInfo.intervalTimer > 0.0f) {
		playerData_.characterInfo.stepBoostInfo.intervalTimer -= deltaTime_;
	}

	// StepBoost入力判定を最初に追加
	if (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING ||
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING) {

		//オーバーヒート状態のチェック
		if (playerData_.characterInfo.overHeatInfo.isOverheated == false) {
			//StepBoost入力判定
			// LTボタン＋スティック入力で発動
			if (inputProvider_->RequestStepBoost()) {
				RequestActiveBoostEffect();
				behaviorManager_->RequestBehavior(GameCharacterBehavior::STEPBOOST);
			}
			//Jump入力判定
			//RTで発動
			if (playerData_.characterInfo.onGround == true) {

				if (inputProvider_->RequestJumpInput()) {
					//ジャンプのリクエスト
					behaviorManager_->RequestBehavior(GameCharacterBehavior::JUMP);
					playerData_.characterInfo.onGround = false; // ジャンプしたので地上ではない
				}
			}
		}
	}

	//移動方向の取得
	playerData_.characterInfo.moveDirection = inputProvider_->GetMoveDirection();
	//カメラ方向のベクトルを取得
	camera_->SetStick(inputProvider_->GetCameraRotateInput());

	//Behaviorの更新
	behaviorManager_->Update(playerData_.characterInfo);

	//エネルギーの更新
	UpdateEnergy();
	//アクティブブーストエフェクトのリクエスト
	RequestActiveBoostEffect();

	// 地面に着地したらRUNNINGに戻る
	if (playerData_.characterInfo.onGround == true &&
		(behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP ||
			behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING)) {
		behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
	} else if (playerData_.characterInfo.onGround == false &&
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING) {
		//空中にいる場合はFLOATINGに切り替え
		behaviorManager_->RequestBehavior(GameCharacterBehavior::FLOATING);
	}

	//攻撃処理
	if (playerData_.characterInfo.isAlive == true) {
		//生存時のみ攻撃処理を行う
		UpdateAttack();
	}

	//HPが0以下なら死亡処理
	if (playerData_.characterInfo.health <= 0.0f) {
		//死亡状態のリクエスト
		playerData_.characterInfo.isAlive = false;
		behaviorManager_->RequestBehavior(GameCharacterBehavior::DEAD);
	}

	//歩行時の煙エミッターのON/OFF制御
	if (playerData_.characterInfo.onGround && (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING ||
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::STEPBOOST)) {
		backEmitter_->SetIsEmit(true);
	} else {
		backEmitter_->SetIsEmit(false);
	}


	//モデルの回転処理
	if (isUseWeapon_) {
		//武器使用中はカメラの向きに合わせる
		Quaternion targetRotate = camera_->GetRotate();
		playerData_.characterInfo.transform.rotate = Easing::Slerp(playerData_.characterInfo.transform.rotate, targetRotate, 0.25f);
		playerData_.characterInfo.transform.rotate = QuaternionMath::Normalize(playerData_.characterInfo.transform.rotate);
	} else {
		if (playerData_.characterInfo.moveDirection.x != 0.0f || playerData_.characterInfo.moveDirection.z != 0.0f) {
			//移動方向に合わせて回転
			float targetAngle = atan2(playerData_.characterInfo.moveDirection.x, playerData_.characterInfo.moveDirection.z);
			Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, targetAngle);
			playerData_.characterInfo.transform.rotate = Easing::Slerp(playerData_.characterInfo.transform.rotate, targetRotate, 0.05f);
			playerData_.characterInfo.transform.rotate = QuaternionMath::Normalize(playerData_.characterInfo.transform.rotate);
		}
	}

	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::ToEuler(playerData_.characterInfo.transform.rotate);
	//カメラの設定
	
	camera_->SetFollowTargetPos(*object3d_->GetModel()->GetSkeleton()->GetJointPosition("neck", object3d_->GetWorldMatrix()));
	camera_->SetFollowTargetRot(eulerRotate);
	camera_->SetFocusTargetPos(playerData_.characterInfo.focusTargetPos);

	//オブジェクトの更新
	object3d_->SetTranslate(playerData_.characterInfo.transform.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->SetScale(playerData_.characterInfo.transform.scale);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//背部エミッターの更新
	std::optional<Vector3> backpackPosition = object3d_->GetModel()->GetSkeleton()->GetJointPosition("leg", object3d_->GetWorldMatrix());
	backEmitter_->SetTranslate(backpackPosition.value());
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("WalkSmoke2")->SetEmitterPosition(backpackPosition.value());
	//backEmitter_->Update();

	//武器の更新
	for (const auto& weapon : weapons_) {
		if (weapon) {
			weapon->SetTarget(playerData_.characterInfo.focusTargetPos);
			weapon->SetTargetVelocity(focusTargetVelocity_);
			weapon->Update();
		}
	}

	//ブーストエフェクトの更新
	for (const auto& boostEffect : boostEffects_) {
		boostEffect->Update();
		boostEffect->SetBehavior(behaviorManager_->GetCurrentBehaviorType());
	}

	playerData_.characterInfo.onGround = false; // 毎フレームリセットし、衝突判定で更新されるようにする
}

//===================================================================================
// ImGuiの更新処理
//===================================================================================

void Player::UpdateImGui() {
#ifdef _DEBUG

	ImGui::Begin("Player");
	ImGui::DragFloat3("Translate", &playerData_.characterInfo.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &playerData_.characterInfo.transform.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &playerData_.characterInfo.transform.rotate.x, 0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &playerData_.characterInfo.health, 1.0f, 0.0f, playerData_.characterInfo.maxHealth);
	ImGui::DragFloat("Energy", &playerData_.characterInfo.energyInfo.energy, 1.0f, 0.0f, playerData_.characterInfo.energyInfo.maxEnergy);
	ImGui::DragFloat3("Velocity", &playerData_.characterInfo.velocity.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &playerData_.characterInfo.moveDirection.x, 0.01f);
	ImGui::Checkbox("OnGround", &playerData_.characterInfo.onGround);

	object3d_->UpdateImGui("Player Object3d");

	// デバッグ用ダメージボタン
	if(ImGui::Button("Damage 1000")) {
		playerData_.characterInfo.health -= 1000.0f;
	}
	// プレイヤーデータの保存ボタン
	if(ImGui::Button("Save Player Data")) {
		SavePlayerData("Player");
	}
	ImGui::Separator();
	// 状態管理マネージャのImGui更新
	behaviorManager_->UpdateImGui();
	// コライダーのImGui更新
	collider_->UpdateImGui("Player");
	// ブーストエフェクトのImGui更新
	for (int i = 0; i < chargeShootableUnits_.size(); i++) {
		boostEffects_[i]->UpdateImGui(magic_enum::enum_name(static_cast<BoostEffectPosition>(i)).data());
	}
	// 武器のImGui更新
	weapons_[0]->UpdateImGui();
	weapons_[1]->UpdateImGui();
	weapons_[2]->UpdateImGui();
	weapons_[3]->UpdateImGui();
	ImGui::End();

#endif // _DEBUG
}

//===================================================================================
// 描画処理
//===================================================================================

void Player::Draw() {
	object3d_->Draw();
	for (const auto& weapon : weapons_) {
		if (weapon) {
			weapon->Draw();
		}
	}

}

//===================================================================================
// ブーストエフェクトの描画処理
//===================================================================================
void Player::DrawBoostEffect() {
	for (const auto& effect : boostEffects_) {
		effect->Draw();
	}
}

//===================================================================================
// プレイヤーデータの読み込み・保存処理
//===================================================================================
void Player::LoadPlayerData(const std::string& characterName) {

	//Jsonからデータを読み込み
	playerData_.characterInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<PlayableCharacterInfo>(characterName);
}

void Player::SavePlayerData(const std::string& characterName) {

	//Jsonにデータを保存
	playerData_.characterInfo.characterName = characterName;
	playerData_.characterInfo.modelFilePath = object3d_->GetModelFilePath();
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<PlayableCharacterInfo>(characterName + ".json", playerData_.characterInfo);
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
		playerData_.characterInfo.health -= bullet->GetDamage();
	}
	if (other->GetCharacterType() == CharacterType::ENEMY_MISSILE) {
		//敵のミサイルに当たった場合
		collider_->SetColor({ 1.0f,0.0f,0.0f,1.0f });
		//ダメージを受ける
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		playerData_.characterInfo.health -= missile->GetDamage();
	}

	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//playerのColliderが接触している面で判断する
		if (collider_->GetSurfaceType() == SurfaceType::BOTTOM) {

			//playerが下面で接触している場合は地面にいると判定
			playerData_.characterInfo.onGround = true;

			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				//貫通している分だけ押し戻す
				//過剰な押し戻しを防ぐために、penetrationDepthを2で割ると良い感じになる
				playerData_.characterInfo.transform.translate += -normal * penetrationDepth * 0.5f;

				//接触面方向の速度を打ち消す
				float velocityAlongNormal = Vector3Math::Dot(playerData_.characterInfo.velocity, normal);
				playerData_.characterInfo.velocity -= normal * velocityAlongNormal;
			}

			collider_->SetColor({ 0.0f,1.0f,0.0f,1.0f }); // コライダーの色を緑に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::WALL) {

			//側面で接触した場合
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				//貫通している分だけ押し戻す
				playerData_.characterInfo.transform.translate += -normal * penetrationDepth;

				//接触面方向の速度を打ち消す
				float dot = Vector3Math::Dot(playerData_.characterInfo.velocity, normal);
				playerData_.characterInfo.velocity -= normal * dot;
			}

			collider_->SetColor({ 0.0f,0.0f,1.0f,1.0f }); // コライダーの色を青に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::TOP) {
			//天井に接触した場合
			if (playerData_.characterInfo.velocity.y > 0.0f) {
				playerData_.characterInfo.velocity.y = 0.0f;
			}

			collider_->SetColor({ 1.0f,1.0f,0.0f,1.0f }); // コライダーの色を黄色に変更
		} 
	}
}

//===================================================================================
//　攻撃処理
//===================================================================================

void Player::UpdateAttack() {

	WeaponAttack(CharacterActionInput::ATTACK_LA); // 1つ目の武器の攻撃
	WeaponAttack(CharacterActionInput::ATTACK_RA); // 2つ目の武器の攻撃
	WeaponAttack(CharacterActionInput::ATTACK_LB); // 3つ目の武器の攻撃
	WeaponAttack(CharacterActionInput::ATTACK_RB); // 4つ目の武器の攻撃

	//チャージ攻撃可能なユニットの処理
	for (int i = 0; i < chargeShootableUnits_.size(); i++) {
		if (chargeShootableUnits_[i] == true) {
			//チャージ撃ち可能なユニットの処理
			auto* weapon = weapons_[i].get();
			if (playerData_.characterInfo.isChargeShooting == true) {
				// チャージ撃ち中の処理
				chargeShootTimer_ -= deltaTime_;
				if (chargeShootTimer_ <= 0.0f) {
					weapon->Attack();
					playerData_.characterInfo.isChargeShooting = false; // チャージ撃ち中フラグをリセット
					chargeShootTimer_ = 0.0f; // タイマーをリセット
					behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
					chargeShootableUnits_[i] = false; // チャージ撃ち可能なユニットのマークをリセット
				}
			}
		}
	}

	if(isUseWeapon_ == true) {
		//武器を使用した場合は一定時間経過後にリセット
		weaponUseTimer_ += deltaTime_;
		if (weaponUseTimer_ >= weaponUseDuration_) {
			isUseWeapon_ = false;
			weaponUseTimer_ = 0.0f;
		}
	}
}

void Player::WeaponAttack(CharacterActionInput actionInput) {

	int weaponIndex = static_cast<int>(actionInput) - static_cast<int>(CharacterActionInput::ATTACK_RA);
	auto* weapon = weapons_[weaponIndex].get();

	if (inputProvider_->RequestAttack(actionInput) == true) {
		//武器を選択したことを記録
		isUseWeapon_ = true;
		//チャージ攻撃可能な場合
		if (weapon->CanChargeAttack()) {

			//武器のチャージ処理
			weapon->Charge(deltaTime_);
			if (inputProvider_->ReleaseAttackInput(actionInput) == true) {
				//チャージ攻撃実行
				weapon->ChargeAttack();
				if (weapon->StopShootOnly()) {
					// 停止撃ち専用の場合はチャージ後に硬直状態へ
					behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
				} else {
					// 移動撃ち可能な場合はRUNNINGに戻す
					behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
				}
			}
		} else {
			//チャージ攻撃不可:通常攻撃
			if (weapon->StopShootOnly() && weapon->GetAttackInterval() <= 0.0f) {
				// 停止撃ち専用:硬直処理を行う
				playerData_.characterInfo.isChargeShooting = true; // チャージ撃ち中フラグを立てる
				chargeShootTimer_ = chargeShootDuration_; // チャージ撃ちのタイマーを設定
				chargeShootableUnits_[weaponIndex] = true; // チャージ撃ち可能なユニットとしてマーク

			} else {
				// 移動撃ち可能
				weapon->Attack();
			}
		}
	} else if (inputProvider_->ReleaseAttackInput(actionInput) == true) {
		// LBボタンが離された場合

		if (weapon->IsCharging()) {
			// チャージ中の場合はチャージ攻撃を終了
			weapon->ChargeAttack();
			if (weapon->StopShootOnly()) {
				// 停止撃ち専用の場合はチャージ後に硬直状態へ
				behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
			} else {
				// 移動撃ち可能な場合はRUNNINGに戻す
				behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
			}
		}
	}


}

//===================================================================================
// エネルギーの更新処理
//===================================================================================

void Player::UpdateEnergy() {

	float& overheatTimer = playerData_.characterInfo.overHeatInfo.overheatTimer; // オーバーヒートタイマー
	float maxEnergy = playerData_.characterInfo.energyInfo.maxEnergy; // 最大エネルギー

	//オーバーヒートしているかどうか
	if (!playerData_.characterInfo.overHeatInfo.isOverheated) {

		if (playerData_.characterInfo.energyInfo.energy <= 0.0f) {
			// エネルギーが0以下になったらオーバーヒート状態にする
			playerData_.characterInfo.overHeatInfo.isOverheated = true;
			overheatTimer = playerData_.characterInfo.overHeatInfo.overheatDuration; // オーバーヒートのタイマーを設定
			playerData_.characterInfo.energyInfo.energy = 0.0f; // エネルギーを0にする
		}

		// エネルギーの回復
		if (playerData_.characterInfo.energyInfo.energy < maxEnergy) {

			//浮遊状態,ジャンプ時、ステップブースト時はエネルギーを回復しない
			if (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING ||
				behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP ||
				behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::STEPBOOST) {
				return; // エネルギーの回復を行わない
			}

			playerData_.characterInfo.energyInfo.energy += playerData_.characterInfo.energyInfo.recoveryRate * deltaTime_;
			if (playerData_.characterInfo.energyInfo.energy > maxEnergy) {
				playerData_.characterInfo.energyInfo.energy = maxEnergy;
			}
		}

	} else {

		// オーバーヒートの処理
		if (overheatTimer > 0.0f) {
			overheatTimer -= deltaTime_;
			if (overheatTimer <= 0.0f) {
				// オーバーヒートが終了したらエネルギーを半分回復
				playerData_.characterInfo.energyInfo.energy = maxEnergy / 2.0f;
				// オーバーヒート状態を解除
				playerData_.characterInfo.overHeatInfo.isOverheated = false;
			}
		}

	}
}

void Player::RequestActiveBoostEffect() {

	//ステップブーストの方向とスティックの向きによってアクティブにするエフェクトを変更
	Vector3 moveDir = playerData_.characterInfo.moveDirection;
	if (moveDir.Length() <= 0.1f) return;

	// --- プレイヤーの向きをQuaternionから取得 ---
	Vector3 localForward = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 playerForward = QuaternionMath::RotateVector(localForward, playerData_.characterInfo.transform.rotate);
	playerForward.y = 0.0f; // 水平方向だけ見る
	playerForward = Vector3Math::Normalize(playerForward);

	// --- スティック方向との角度差を求める ---
	float dot = Vector3Math::Dot(playerForward, moveDir);
	float crossY = playerForward.x * moveDir.z - playerForward.z * moveDir.x;
	float angle = atan2(crossY, dot) * (180.0f / std::numbers::pi_v<float>); // -180°～180°

	// --- エフェクト制御 ---
	if (angle >= -45.0f && angle < 45.0f) {
		// 前方向
		boostEffects_[LEFT_BACK]->SetIsActive(true);
		boostEffects_[RIGHT_BACK]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(false);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(false);
	}
	else if (angle >= 45.0f && angle < 135.0f) {
		// 右方向
		boostEffects_[LEFT_BACK]->SetIsActive(false);
		boostEffects_[RIGHT_BACK]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(false);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(true);
	}
	else if (angle <= -45.0f && angle > -135.0f) {
		// 左方向
		boostEffects_[LEFT_BACK]->SetIsActive(true);
		boostEffects_[RIGHT_BACK]->SetIsActive(false);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(true);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(false);
	}
	else {
		// 後方向
		boostEffects_[LEFT_BACK]->SetIsActive(true);
		boostEffects_[RIGHT_BACK]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(false);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(false);
	}
}
