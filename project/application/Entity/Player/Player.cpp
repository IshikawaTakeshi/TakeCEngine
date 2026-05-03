#include "Player.h"
#include "engine/3d/Model.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/Input/Input.h"
#include "engine/Math/Easing.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/Vector3Math.h"
#include "engine/Utility/JsonLoader.h"
#include "engine/Utility/StringUtility.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"

#include "application/Effect/BoostEffectPositionEnum.h"
#include "application/Entity/WeaponUnit.h"
#include "application/Weapon/Bazooka/Bazooka.h"
#include "application/Weapon/Launcher/VerticalMissileLauncher.h"
#include "application/Weapon/MachineGun/MachineGun.h"
#include "application/Weapon/Rifle/Rifle.h"
#include "application/Weapon/ShotGun/ShotGun.h"
#include "application/Tool/BreakGaugeUtil.h"


//===================================================================================
// 初期化処理
//===================================================================================

void Player::Initialize(Object3dCommon* object3dCommon,
	const std::string& filePath) {

	// キャラクタータイプ設定
	characterType_ = CharacterType::PLAYER;

	// PlayerDataの読み込み
	playerData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<CharacterData>(
		"PlayerData.json");
	playerData_.characterInfo.transform = {
		{1.5f, 1.5f, 1.5f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -30.0f} };

	// オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	object3d_->SetScale(playerData_.characterInfo.transform.scale);
	object3d_->SetUseExternalAnimation(
		true); // 外部からアニメーションを設定するようにする

	// コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetOwner(this);
	collider_->SetOffset(playerData_.characterInfo.colliderInfo
		.offset); // コライダーのオフセットを設定
	collider_->SetHalfSize(playerData_.characterInfo.colliderInfo
		.halfSize); // コライダーの半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Player));

	camera_ = object3dCommon->GetDefaultCamera();
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	weapons_.resize(WeaponUnit::Size);
	weaponTypes_.resize(WeaponUnit::Size);
	for (size_t i = 0; i < weaponTypes_.size(); i++) {
		weaponTypes_[i] = playerData_.weaponData[i].weaponType;
	}
	chargeShootableUnits_.resize(4);

	// ブーストエフェクトの初期化
	boostEffects_.resize(kNumPositions);
	for (int i = 0; i < kNumPositions; i++) {
		boostEffects_[i] = std::make_unique<BoostEffect>();
		boostEffects_[i]->Initialize(this,
			"BoostEffect_Player.json", 
			"BoostEffect_Appear_Player.json",
			"StepBoostEffect_Appear_Player.json");
	}
	boostEffects_[LEFT_LEG]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(), "knees_left.002");
	boostEffects_[RIGHT_LEG]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(), "knees_right.002");
	boostEffects_[RIGHT_SHOULDER]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(), "body_joint_right.002");
	boostEffects_[LEFT_SHOULDER]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(), "body_joint_left.002");
	boostEffects_[BACKPACK]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(), "backpack.001");

	// BehaviorManagerの初期化
	stateManager_ = std::make_unique<GameCharacterStateManager>();
	stateManager_->Initialize(inputProvider_);
	stateManager_->InitializeStates(playerData_.characterInfo);

	// アニメーションマッパーの登録
	animationMapper_.Register(
		GameCharacterState::RUNNING,
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Running"),
		0.2f);
	animationMapper_.Register(
		GameCharacterState::FLOATING,
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Floating"),
		0.2f);
	animationMapper_.Register(
		GameCharacterState::JUMP,
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Jump"),
		0.15f);
	animationMapper_.Register(
		GameCharacterState::STEPBOOST,
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Running"),
		0.1f);
	animationMapper_.Register(
		GameCharacterState::CHARGESHOOT_STUN,
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Running"),
		0.2f);
	animationMapper_.Register(
		GameCharacterState::DEAD,
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Running"),
		0.3f, false);

	// BehaviorManagerにアニメーションコンポーネントを設定
	stateManager_->SetAnimationComponents(object3d_->GetAnimatorController(),
		&animationMapper_);
}

//===================================================================================
// 武器の初期化処理
//===================================================================================

void Player::WeaponInitialize(Object3dCommon* object3dCommon,
	BulletManager* bulletManager) {
	// 武器の初期化
	for (int i = 0; i < weapons_.size(); i++) {
		if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_RIFLE) {
			// ライフルの武器を初期化
			weapons_[i] = std::make_unique<Rifle>();

		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_BAZOOKA) {
			weapons_[i] = std::make_unique<Bazooka>();

		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_VERTICAL_MISSILE) {
			// 垂直ミサイルの武器を初期化
			weapons_[i] = std::make_unique<VerticalMissileLauncher>();

		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_MACHINE_GUN) {
			// マシンガンの武器を初期化
			weapons_[i] = std::make_unique<MachineGun>();

		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_SHOTGUN) {
			// ショットガンの武器を初期化
			weapons_[i] = std::make_unique<ShotGun>();

		} else {
			// 武器が設定されていない場合はnullptrのまま
			weapons_[i] = nullptr;
		}
		weapons_[i]->Initialize(object3dCommon, bulletManager);
		weapons_[i]->SetOwnerObject(this);
	}

	weapons_[R_ARMS]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(),
		"weaponJointPoint_RB.tip"); // 1つ目の武器を右手に取り付け
	weapons_[R_ARMS]->SetUnitPosition(
		R_ARMS); // 1つ目の武器のユニットポジションを設定
	weapons_[L_ARMS]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(),
		"weaponJointPoint_LB.tip"); // 2つ目の武器を左手に取り付け
	weapons_[L_ARMS]->SetUnitPosition(
		L_ARMS); // 2つ目の武器のユニットポジションを設定
	weapons_[R_BACK]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(),
		"weaponJointPoint_RT.tip"); // 3つ目の武器を背中に取り付け
	weapons_[R_BACK]->SetUnitPosition(
		R_BACK); // 3つ目の武器のユニットポジションを設定
	weapons_[L_BACK]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(),
		"weaponJointPoint_LT.tip"); // 4つ目の武器を背中に取り付け
	weapons_[L_BACK]->SetUnitPosition(
		L_BACK); // 4つ目の武器のユニットポジションを設定
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

	// ブレイクゲージの自然減少（スタン中は減少しない）
	auto& gaugeInfo = playerData_.characterInfo.breakGaugeInfo;


	// EventManagerへ通知
	TakeCFrameWork::GetEventManager()->PostEvent("BreakGaugeUpdate_Player", &gaugeInfo);

	if (gaugeInfo.isStunned) {
		gaugeInfo.entries.clear();
	} else {
		BreakGaugeUtil::UpdateBreakGaugeEntries(gaugeInfo);
		// スタン猶予タイマーの更新
		gaugeInfo.stunGraceTimer.Update();

		if (gaugeInfo.breakGauge >= gaugeInfo.maxBreakGauge) {
			// スタン開始：entriesをクリア
			gaugeInfo.entries.clear();
			gaugeInfo.isStunned = true;

			// state遷移をリクエスト
			stateManager_->RequestState(GameCharacterState::BREAK_STUN);
		}
	}

	// stepBoostのインターバルの更新
	playerData_.characterInfo.stepBoostInfo.interval =
		0.2f; // ステップブーストのインターバル
	if (playerData_.characterInfo.stepBoostInfo.intervalTimer > 0.0f) {
		playerData_.characterInfo.stepBoostInfo.intervalTimer -= deltaTime_;
	}

	if (playerData_.characterInfo.isInCombat) {
		// StepBoost入力判定を最初に追加
		if (stateManager_->GetCurrentStateType() == GameCharacterState::RUNNING ||
			stateManager_->GetCurrentStateType() == GameCharacterState::FLOATING) {

			// オーバーヒート状態のチェック
			if (playerData_.characterInfo.overHeatInfo.isOverheated == false) {
				// StepBoost入力判定
				//  LTボタン＋スティック入力で発動
				if (inputProvider_->RequestStepBoost()) {
					stateManager_->RequestState(GameCharacterState::STEPBOOST);
					TakeCFrameWork::GetPostEffectManager()->PlayEffect("RadialBlur");
				}
				// Jump入力判定
				// RTで発動
				if (playerData_.characterInfo.onGround == true) {

					if (inputProvider_->RequestJumpInput()) {
						// ジャンプのリクエスト
						stateManager_->RequestState(GameCharacterState::JUMP);
						playerData_.characterInfo.onGround =
							false; // ジャンプしたので地上ではない
					}
				}
			}
		}

		// 移動方向の取得
		playerData_.characterInfo.moveDirection =
			inputProvider_->GetMoveDirection();
		// カメラ方向のベクトルを取得
		camera_->SetStick(inputProvider_->GetCameraRotateInput());

		if (inputProvider_->RequestChangeCameraMode()) {
			camera_->SetRequestedChangeCameraMode(true);
			isFocus_ = !isFocus_;
		}

		// 攻撃処理
		if (playerData_.characterInfo.isAlive == true && 
			(stateManager_->GetCurrentStateType() == GameCharacterState::BREAK_STUN) == false) {
			// 生存時のみ攻撃処理を行う
			UpdateAttack();
		}
	}

	// エネルギーの更新
	UpdateEnergy();
	// アクティブブーストエフェクトのリクエスト
	RequestActiveBoostEffect();
	// Behaviorの更新
	stateManager_->Update(playerData_.characterInfo);

	// 地面に着地したらRUNNINGに戻る
	if (playerData_.characterInfo.onGround == true &&
		(stateManager_->GetCurrentStateType() == GameCharacterState::JUMP ||
			stateManager_->GetCurrentStateType() == GameCharacterState::FLOATING)) {
		stateManager_->RequestState(GameCharacterState::RUNNING);
	} else if (playerData_.characterInfo.onGround == false &&
		stateManager_->GetCurrentStateType() ==
		GameCharacterState::RUNNING) {
		// 空中にいる場合はFLOATINGに切り替え
		stateManager_->RequestState(GameCharacterState::FLOATING);
	}

	// HPが0以下なら死亡処理
	if (playerData_.characterInfo.health <= 0.0f) {
		// 死亡状態のリクエスト
		playerData_.characterInfo.isAlive = false;
		stateManager_->RequestState(GameCharacterState::DEAD);
	}

	// モデルの回転処理
	Quaternion targetRotate;
	Quaternion& characterRotate = playerData_.characterInfo.transform.rotate;
	if (camera_->GetCameraState() == Camera::GameCameraState::LOCKON ||
		camera_->GetCameraState() == Camera::GameCameraState::ENEMY_DESTROYED) {
		// ロックオン中はフォーカス対象の方向に回転
		Vector3 toTarget = playerData_.characterInfo.focusTargetPos -
			playerData_.characterInfo.transform.translate;
		toTarget.y = 0.0f; // y成分を無視
		toTarget = Vector3Math::Normalize(toTarget);
		float targetAngle = atan2(toTarget.x, toTarget.z);
		targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion(
			{ 0.0f, 1.0f, 0.0f }, targetAngle);

		characterRotate = Easing::Slerp(characterRotate, targetRotate, 0.9f);
		characterRotate = QuaternionMath::Normalize(characterRotate);
	} else {
		if (playerData_.characterInfo.moveDirection.x != 0.0f ||
			playerData_.characterInfo.moveDirection.z != 0.0f) {
			// 移動方向に合わせて回転
			float targetAngle = atan2(playerData_.characterInfo.moveDirection.x,
				playerData_.characterInfo.moveDirection.z);
			targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion(
				{ 0.0f, 1.0f, 0.0f }, targetAngle);
			characterRotate = Easing::Slerp(characterRotate, targetRotate, 0.05f);
			characterRotate = QuaternionMath::Normalize(characterRotate);
		}
	}

	// 首のジョイント位置を体の位置として取得
	auto jointWorldMatrixOpt =
		object3d_->GetModel()->GetSkeleton()->GetJointWorldMatrix(
			"neck", object3d_->GetWorldMatrix());
	bodyPosition_ = { jointWorldMatrixOpt->m[3][0], jointWorldMatrixOpt->m[3][1],
					 jointWorldMatrixOpt->m[3][2] };

	// Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::ToEuler(characterRotate);
	// カメラの設定
	camera_->SetFollowTargetPos(
		*object3d_->GetModel()->GetSkeleton()->GetJointPosition(
			"neck", object3d_->GetWorldMatrix()));
	camera_->SetFollowTargetRot(eulerRotate);
	camera_->SetFocusTargetPos(playerData_.characterInfo.focusTargetPos);

	// オブジェクトの更新
	object3d_->SetTranslate(playerData_.characterInfo.transform.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->SetScale(playerData_.characterInfo.transform.scale);
	// アニメーションコントローラの更新（object3d_->Update()より前に呼ぶ）
	object3d_->GetAnimatorController()->Update(deltaTime_);
	object3d_->Update();
	object3d_->GetModel()->UpdateSkinningFromSkeleton();
	collider_->Update(object3d_.get());

	// 武器の更新
	for (const auto& weapon : weapons_) {
		if (weapon) {
			weapon->SetTarget(playerData_.characterInfo.focusTargetPos);
			weapon->SetTargetVelocity(focusTargetVelocity_);
			weapon->Update();
		}
	}

	// ブーストエフェクトの更新
	for (const auto& boostEffect : boostEffects_) {
		boostEffect->Update();
		boostEffect->SetCharacterState(stateManager_->GetCurrentStateType());
	}

	playerData_.characterInfo.onGround = false; // 毎フレームリセットし、衝突判定で更新されるようにする
}

//===================================================================================
// ImGuiの更新処理
//===================================================================================

void Player::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)

	ImGui::Begin("Player");
	ImGui::DragFloat3("Translate",
		&playerData_.characterInfo.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &playerData_.characterInfo.transform.scale.x,
		0.01f);
	ImGui::DragFloat4("Rotate", &playerData_.characterInfo.transform.rotate.x,
		0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &playerData_.characterInfo.health, 1.0f, 0.0f,
		playerData_.characterInfo.maxHealth);
	ImGui::DragFloat("Energy", &playerData_.characterInfo.energyInfo.energy, 1.0f,
		0.0f, playerData_.characterInfo.energyInfo.maxEnergy);
	ImGui::DragFloat3("Velocity", &playerData_.characterInfo.velocity.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &playerData_.characterInfo.moveDirection.x,
		0.01f);
	ImGui::Checkbox("OnGround", &playerData_.characterInfo.onGround);
	ImGui::SeparatorText("Collider Info");
	ImGui::DragFloat3("offset", &playerData_.characterInfo.colliderInfo.offset.x,
		0.01f);
	ImGui::DragFloat3("halfSize",
		&playerData_.characterInfo.colliderInfo.halfSize.x, 0.01f);

	object3d_->UpdateImGui("Player Object3d");

	// デバッグ用ダメージボタン
	if (ImGui::Button("Damage 1000")) {
		playerData_.characterInfo.health -= 1000.0f;
	}
	// プレイヤーデータの保存ボタン
	if (ImGui::Button("Save Player Data")) {
		SavePlayerData("Player");
	}
	ImGui::Separator();
	// 状態管理マネージャのImGui更新
	stateManager_->UpdateImGui();
	// コライダーのImGui更新
	collider_->SetOffset(playerData_.characterInfo.colliderInfo.offset);
	collider_->SetHalfSize(playerData_.characterInfo.colliderInfo.halfSize);
	collider_->UpdateImGui("Player");
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

void Player::DrawShadow(const LightCameraInfo& lightCamera) {
	object3d_->DrawShadow(lightCamera);
	for (const auto& weapon : weapons_) {
		if (weapon) {
			weapon->DrawShadow(lightCamera);
		}
	}
}

//===================================================================================
// プレイヤーデータの読み込み・保存処理
//===================================================================================
void Player::LoadPlayerData(const std::string& characterName) {

	// Jsonからデータを読み込み
	playerData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<CharacterData>(
		characterName);
}

void Player::SavePlayerData(const std::string& characterName) {

	// Jsonにデータを保存
	playerData_.characterInfo.characterName = characterName;
	playerData_.characterInfo.modelFilePath = object3d_->GetModelFilePath();
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<CharacterData>(
		"PlayerData.json", playerData_);
}

void Player::DrawCollider() {
#if defined(_DEBUG) || defined(_DEVELOP)
	collider_->DrawCollider();
#endif
}

//===================================================================================
// 衝突時の処理
//===================================================================================

void Player::OnCollisionAction(GameCharacter* other) {

	if (other->GetCharacterType() == CharacterType::ENEMY) {
		// 衝突時の処理
		collider_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	}
	if (other->GetCharacterType() == CharacterType::ENEMY_BULLET) {
		// 敵の弾に当たった場合
		collider_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		// ダメージを受ける
		Bullet* bullet = dynamic_cast<Bullet*>(other);
		playerData_.characterInfo.health -= bullet->GetDamage();
		// ブレイクゲージを蓄積
		AccumulateBreakGauge(bullet->GetDamage());
		//エフェクトの再生
		TakeCFrameWork::GetPostEffectManager()->PlayEffect("DamageHit");
	}
	if (other->GetCharacterType() == CharacterType::ENEMY_MISSILE) {
		// 敵のミサイルに当たった場合
		collider_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		// ダメージを受ける
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		playerData_.characterInfo.health -= missile->GetDamage();
		// ブレイクゲージを蓄積
		AccumulateBreakGauge(missile->GetDamage());
		//エフェクトの再生
		TakeCFrameWork::GetPostEffectManager()->PlayEffect("DamageHit");
	}

	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		// playerのColliderが接触している面で判断する
		if (collider_->GetSurfaceType() == SurfaceType::BOTTOM) {

			// playerが下面で接触している場合は地面にいると判定
			playerData_.characterInfo.onGround = true;

			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				// 貫通している分だけ押し戻す
				// 過剰な押し戻しを防ぐために、penetrationDepthを2で割ると良い感じになる
				playerData_.characterInfo.transform.translate +=
					-normal * penetrationDepth * 0.5f;

				// 接触面方向の速度を打ち消す
				float velocityAlongNormal =
					Vector3Math::Dot(playerData_.characterInfo.velocity, normal);
				playerData_.characterInfo.velocity -= normal * velocityAlongNormal;
			}

			collider_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // コライダーの色を緑に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::WALL) {

			// 側面で接触した場合
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				// 貫通している分だけ押し戻す
				playerData_.characterInfo.transform.translate +=
					-normal * penetrationDepth;

				// 接触面方向の速度を打ち消す
				float dot =
					Vector3Math::Dot(playerData_.characterInfo.velocity, normal);
				playerData_.characterInfo.velocity -= normal * dot;
			}

			collider_->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f }); // コライダーの色を青に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::TOP) {
			// 天井に接触した場合
			if (playerData_.characterInfo.velocity.y > 0.0f) {
				playerData_.characterInfo.velocity.y = 0.0f;
			}

			collider_->SetColor(
				{ 1.0f, 1.0f, 0.0f, 1.0f }); // コライダーの色を黄色に変更
		}
	}
}

//===================================================================================
// 　攻撃処理
//===================================================================================

void Player::UpdateAttack() {

	WeaponAttack(CharacterActionInput::ATTACK_LA); // 1つ目の武器の攻撃
	WeaponAttack(CharacterActionInput::ATTACK_RA); // 2つ目の武器の攻撃
	WeaponAttack(CharacterActionInput::ATTACK_LB); // 3つ目の武器の攻撃
	WeaponAttack(CharacterActionInput::ATTACK_RB); // 4つ目の武器の攻撃

	// チャージ攻撃可能なユニットの処理
	for (int i = 0; i < chargeShootableUnits_.size(); i++) {
		if (chargeShootableUnits_[i] == true) {
			// チャージ撃ち可能なユニットの処理
			auto* weapon = weapons_[i].get();

			if (playerData_.characterInfo.isChargeShooting == true) {
				// チャージ撃ち中の処理
				chargeShootTimer_.Update();
				if (chargeShootTimer_.IsFinished()) {
					weapon->Attack();
					camera_->RequestShake(ShakeCameraMode::VERTICAL, 0.5f,
						3.0f); // カメラシェイクをリクエスト

					playerData_.characterInfo.isChargeShooting =
						false; // チャージ撃ち中フラグをリセット
					chargeShootTimer_.Stop();
					stateManager_->RequestState(GameCharacterState::CHARGESHOOT_STUN);
					chargeShootableUnits_[i] =
						false; // チャージ撃ち可能なユニットのマークをリセット
				}
			}
		}
	}

	if (isUseWeapon_ == true) {
		// 武器を使用した場合は一定時間経過後にリセット
		weaponUseTimer_ += deltaTime_;
		if (weaponUseTimer_ >= weaponUseDuration_) {
			isUseWeapon_ = false;
			weaponUseTimer_ = 0.0f;
		}
	}
}

void Player::WeaponAttack(CharacterActionInput actionInput) {

	int weaponIndex = static_cast<int>(actionInput) -
		static_cast<int>(CharacterActionInput::ATTACK_LA);
	auto* weapon = weapons_[weaponIndex].get();

	if (inputProvider_->RequestAttack(actionInput) == true) {
		// 武器を選択したことを記録
		isUseWeapon_ = true;
		// チャージ攻撃可能な場合
		if (weapon->CanChargeAttack()) {

			// 武器のチャージ処理
			weapon->Charge(deltaTime_);
			if (inputProvider_->ReleaseAttackInput(actionInput) == true) {
				// チャージ攻撃実行
				weapon->ChargeAttack();

				if (weapon->StopShootOnly()) {
					// 停止撃ち専用の場合はチャージ後に硬直状態へ
					stateManager_->RequestState(GameCharacterState::CHARGESHOOT_STUN);
				} else {
					// 移動撃ち可能な場合はRUNNINGに戻す
					stateManager_->RequestState(GameCharacterState::RUNNING);
				}
			}
		} else {
			// チャージ攻撃不可:通常攻撃
			if (weapon->StopShootOnly() && weapon->GetAttackInterval() <= 0.0f) {
				// 停止撃ち専用:硬直処理を行う
				playerData_.characterInfo.isChargeShooting =
					true; // チャージ撃ち中フラグを立てる
				chargeShootableUnits_[weaponIndex] =
					true; // チャージ撃ち可能なユニットとしてマーク

				// タイマーが終了している場合は初期化
				if (chargeShootTimer_.IsFinished() == true) {
					chargeShootTimer_.Initialize(chargeShootDuration_, 0.0f);
				}

			} else {
				// 移動撃ち可能
				weapon->Attack();
				camera_->RequestShake(ShakeCameraMode::HORIZONTAL, 0.1f,
					1.0f); // カメラシェイクをリクエスト
			}
		}
	} else if (inputProvider_->ReleaseAttackInput(actionInput) == true) {
		// LBボタンが離された場合

		if (weapon->IsCharging()) {
			// チャージ中の場合はチャージ攻撃を終了
			weapon->ChargeAttack();
			if (weapon->StopShootOnly()) {
				// 停止撃ち専用の場合はチャージ後に硬直状態へ
				stateManager_->RequestState(GameCharacterState::CHARGESHOOT_STUN);
			} else {
				// 移動撃ち可能な場合はRUNNINGに戻す
				stateManager_->RequestState(GameCharacterState::RUNNING);
			}
		}
	}
}

//===================================================================================
// エネルギーの更新処理
//===================================================================================

void Player::UpdateEnergy() {

	float& overheatTimer = playerData_.characterInfo.overHeatInfo
		.overheatTimer; // オーバーヒートタイマー
	float maxEnergy =
		playerData_.characterInfo.energyInfo.maxEnergy; // 最大エネルギー

	// オーバーヒートしているかどうか
	if (!playerData_.characterInfo.overHeatInfo.isOverheated) {

		if (playerData_.characterInfo.energyInfo.energy <= 0.0f) {
			// エネルギーが0以下になったらオーバーヒート状態にする
			playerData_.characterInfo.overHeatInfo.isOverheated = true;
			overheatTimer = playerData_.characterInfo.overHeatInfo
				.overheatDuration; // オーバーヒートのタイマーを設定
			playerData_.characterInfo.energyInfo.energy = 0.0f; // エネルギーを0にする
		}

		// エネルギーの回復
		if (playerData_.characterInfo.energyInfo.energy < maxEnergy) {

			// 浮遊状態,ジャンプ時、ステップブースト時はエネルギーを回復しない
			if (stateManager_->GetCurrentStateType() ==
				GameCharacterState::FLOATING ||
				stateManager_->GetCurrentStateType() == GameCharacterState::JUMP ||
				stateManager_->GetCurrentStateType() ==
				GameCharacterState::STEPBOOST) {
				return; // エネルギーの回復を行わない
			}

			playerData_.characterInfo.energyInfo.energy +=
				playerData_.characterInfo.energyInfo.recoveryRate * deltaTime_;
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

	// 毎フレーム一度全OFF（状態残り対策）
	for (const auto& boostEffect : boostEffects_) {
		boostEffect->SetIsActive(false);
	}

	// ステップブーストの方向とスティックの向きによってアクティブにするエフェクトを変更
	Vector3 moveDir = playerData_.characterInfo.moveDirection;

	if (moveDir.Length() <= 0.1f) {
		// ニュートラルなら全OFFのまま終了
		return;
	}

	// 背中のエフェクトは常にアクティブ
	boostEffects_[BACKPACK]->SetIsActive(true);

	// --- プレイヤーの向きをQuaternionから取得 ---
	Vector3 localForward = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 playerForward = QuaternionMath::RotateVector(
		localForward, playerData_.characterInfo.transform.rotate);
	playerForward.y = 0.0f; // 水平方向だけ見る
	playerForward = Vector3Math::Normalize(playerForward);

	// moveDir も正規化して角度判定を安定化
	moveDir.y = 0.0f;
	moveDir = Vector3Math::Normalize(moveDir);

	// --- スティック方向との角度差を求める ---
	float dot = Vector3Math::Dot(playerForward, moveDir);
	float crossY = playerForward.x * moveDir.z - playerForward.z * moveDir.x;
	float angle =
		atan2(crossY, dot) * (180.0f / std::numbers::pi_v<float>); // -180°～180°

	// --- 角度差に応じてエフェクトをアクティブにする ---
	BoostDirection currentDirection = BoostDirection::NONE;

	if (angle <= -45.0f && angle > -135.0f) {
		// 左
		currentDirection = BoostDirection::LEFT;
		boostEffects_[LEFT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_LEG]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(true);
		if (previousBoostDirection_ != currentDirection) {
			boostEffects_[LEFT_SHOULDER]->PlayAppearEffect();
		}

		if(stateManager_->GetNextStateType() == GameCharacterState::STEPBOOST) {
			boostEffects_[LEFT_SHOULDER]->PlayStepBoostEffect();
		}
		
	}
	else if (angle >= 45.0f && angle < 135.0f) {
		// 右
		currentDirection = BoostDirection::RIGHT;
		boostEffects_[LEFT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(true);
		if (previousBoostDirection_ != currentDirection) {
			boostEffects_[RIGHT_SHOULDER]->PlayAppearEffect();
		}

		if(stateManager_->GetNextStateType() == GameCharacterState::STEPBOOST) {
			boostEffects_[RIGHT_SHOULDER]->PlayStepBoostEffect();
		}
	}
	else if (angle > -45.0f && angle < 45.0f) {
		// 前
		currentDirection = BoostDirection::FORWARD;
		boostEffects_[LEFT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_LEG]->SetIsActive(true);
		if (previousBoostDirection_ != currentDirection) {
			boostEffects_[BACKPACK]->PlayAppearEffect();

		}

		if (stateManager_->GetNextStateType() == GameCharacterState::STEPBOOST) {
			boostEffects_[BACKPACK]->PlayStepBoostEffect();
		}
	}
	else {
		// 後ろ
		currentDirection = BoostDirection::BACKWARD;
	}

	previousBoostDirection_ = currentDirection;
}

void Player::RequestAppearBoostEffect() {
	// 登場エフェクトをリクエスト
	for (const auto& boostEffect : boostEffects_) {
		boostEffect->PlayAppearEffect();
	}
}

//===================================================================================
// ブレイクゲージの蓄積処理
//===================================================================================
void Player::AccumulateBreakGauge(float damage) {

	auto& gaugeInfo = playerData_.characterInfo.breakGaugeInfo;

	if (!playerData_.characterInfo.isAlive ||
		gaugeInfo.isStunned ||
		gaugeInfo.stunGraceTimer.IsFinished() == false) {

		// 死亡している、すでにスタンしている、またはスタン猶予タイマーが有効な場合はゲージを蓄積しない
		return; 
	}
	// ダメージに係数を乗算してゲージに加算（係数は武器ごとに変えてもよい）
	gaugeInfo.breakGauge += damage * 0.5f;

	// 被弾履歴(遅延減衰待ち用)を追加
	BreakGaugeEntry entry;
	entry.amount = damage * 0.5f;
	entry.delayTimer.Initialize(2.0f, 0.0f); // 2秒待機後に減衰開始すると仮定
	gaugeInfo.entries.push_back(entry);

	if (gaugeInfo.breakGauge >= gaugeInfo.maxBreakGauge) {
		gaugeInfo.breakGauge = gaugeInfo.maxBreakGauge;
		stateManager_->RequestState(GameCharacterState::BREAK_STUN);
	}
}
