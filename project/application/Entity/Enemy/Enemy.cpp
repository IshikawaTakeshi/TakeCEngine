#include "Enemy.h"
#include "Vector3Math.h"
#include "engine/3d/Model.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/Input/Input.h"
#include "engine/Utility/JsonLoader.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/camera/CameraManager.h"
#include "math/Easing.h"

#include "application/Effect/BoostEffectPositionEnum.h"
#include "application/Entity/WeaponUnit.h"
#include "application/Tool/BreakGaugeUtil.h"
#include "application/Weapon/Bazooka/Bazooka.h"
#include "application/Weapon/Launcher/VerticalMissileLauncher.h"
#include "application/Weapon/MachineGun/MachineGun.h"
#include "application/Weapon/Rifle/Rifle.h"
#include "application/Weapon/ShotGun/ShotGun.h"


//========================================================================================================
// 　初期化
//========================================================================================================
void Enemy::Initialize(Object3dCommon* object3dCommon,
	const std::string& filePath) {

	// キャラクタータイプ設定
	characterType_ = CharacterType::ENEMY;

	// EnemyDataの読み込み
	enemyData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<CharacterData>(
		"EnemyData.json");

	// オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	object3d_->SetScale(enemyData_.characterInfo.transform.scale);
	object3d_->SetUseExternalAnimation(
		true); // 外部からアニメーションを設定するようにする
	// コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetOwner(this);
	collider_->SetOffset(enemyData_.characterInfo.colliderInfo
		.offset); // コライダーのオフセットを設定
	collider_->SetHalfSize(
		enemyData_.characterInfo.colliderInfo.halfSize); // コライダーの半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Enemy));

	// emitter設定
	// emitter0
	particleEmitter_.resize(3);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0", "DamageSpark.json");
	// emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", "DamageSpark2.json");
	// emitter2
	particleEmitter_[2] = std::make_unique<ParticleEmitter>();
	particleEmitter_[2]->Initialize("EnemyEmitter2", "SparkExplosion.json");

	// 死亡エフェクト初期化
	deadEffect_ = std::make_unique<DeadEffect>();
	deadEffect_->Initialize();

	// 武器の初期化
	weapons_.resize(WeaponUnit::Size);
	weaponTypes_.resize(WeaponUnit::Size);
	for (size_t i = 0; i < weaponTypes_.size(); i++) {
		weaponTypes_[i] = enemyData_.weaponData[i].weaponType;
	}
	chargeShootableUnits_.resize(weapons_.size());

	// デルタタイムの取得
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	// bulletSensor_の初期化
	bulletSensor_ = std::make_unique<BulletSensor>();
	bulletSensor_->Initialize(object3dCommon, "ICOBall.gltf");
	bulletSensor_->SetSensorRadius(collider_->GetHalfSize().x * 20.0f);

	// AIBrainSystemの初期化
	aiBrainSystem_ = std::make_unique<AIBrainSystem>();
	aiBrainSystem_->Initialize(&enemyData_.characterInfo, weapons_.size());
	aiBrainSystem_->SetOrbitRadius(orbitRadius_);
	inputProvider_->SetAIBrainSystem(aiBrainSystem_.get());

	// StateManagerの初期化
	stateManager_ = std::make_unique<GameCharacterStateManager>();
	stateManager_->Initialize(inputProvider_);
	stateManager_->InitializeStates(enemyData_.characterInfo);

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
		0.3f);

	// アニメーションコントローラの初期化
	animatorController_.Initialize(object3d_->GetModel()->GetSkeleton());

	// StateManagerにアニメーションコンポーネントを設定
	stateManager_->SetAnimationComponents(&animatorController_,
		&animationMapper_);

	// ブーストエフェクトの初期化
	boostEffects_.resize(kNumPositions);
	for (int i = 0; i < boostEffects_.size(); i++) {
		boostEffects_[i] = std::make_unique<BoostEffect>();
		boostEffects_[i]->Initialize(this, "BoostEffect_Enemy.json");
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

	// Blackboardの生成
	blackboard_ = std::make_unique<Blackboard>();

	// JSONからビヘイビアツリーを構築
	behaviorTree_ = comboFactory_.LoadComboSetData(
		"DefaultComboSet.json",
		stateManager_.get()
	);
}

//========================================================================================================
// 武器の初期化
//========================================================================================================
void Enemy::WeaponInitialize(Object3dCommon* object3dCommon,
	BulletManager* bulletManager) {
	// 武器の初期化
	for (int i = 0; i < weapons_.size(); i++) {
		if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_RIFLE) {
			// ライフルの武器を初期化
			weapons_[i] = std::make_unique<Rifle>();

		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_BAZOOKA) {
			// バズーカの武器を初期化
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
		"weaponJointPoint_LT.tip"); // 3つ目の武器を背中に取り付け
	weapons_[R_BACK]->SetUnitPosition(
		R_BACK); // 3つ目の武器のユニットポジションを設定
	weapons_[L_BACK]->AttachToSkeletonJoint(
		object3d_->GetModel()->GetSkeleton(),
		"weaponJointPoint_RT.tip"); // 4つ目の武器を背中に取り付け
	weapons_[L_BACK]->SetUnitPosition(
		L_BACK); // 4つ目の武器のユニットポジションを設定
}

//========================================================================================================
// 武器の取得
//========================================================================================================
BaseWeapon* Enemy::GetCurrentWeapon(int index) const {
	return weapons_[index].get();
}

std::vector<std::unique_ptr<BaseWeapon>>& Enemy::GetWeapons() {
	return weapons_;
}

//========================================================================================================
// EnemyのGameCharacterContextの読み込み・保存
//========================================================================================================
void Enemy::LoadEnemyData(const std::string& characterJsonPath) {

	// JsonLoaderを使ってEnemyのGameCharacterContextを読み込み
	enemyData_.characterInfo =
		TakeCFrameWork::GetJsonLoader()->LoadJsonData<PlayableCharacterInfo>(
			characterJsonPath);
}

void Enemy::SaveEnemyData(const std::string& characterJsonPath) {

	// JsonLoaderを使ってEnemyのGameCharacterContextを保存
	enemyData_.characterInfo.characterName = characterJsonPath;
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<CharacterData>("EnemyData.json",
		enemyData_);
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Enemy::Update() {

	// --- ブレイクゲージ更新（ここを上の方に追加する認識でOK） ---
	auto& breakGaugeInfo = enemyData_.characterInfo.breakGaugeInfo;


	// EventManagerへ通知
	TakeCFrameWork::GetEventManager()->PostEvent("BreakGaugeUpdate_Enemy", &breakGaugeInfo);

	if (breakGaugeInfo.isStunned) {
		breakGaugeInfo.entries.clear();
	} else {
		BreakGaugeUtil::UpdateBreakGaugeEntries(breakGaugeInfo);

		// スタン猶予タイマーの更新
		breakGaugeInfo.stunGraceTimer.Update();

		if (breakGaugeInfo.breakGauge >= breakGaugeInfo.maxBreakGauge) {
			// スタン開始：entriesをクリア
			breakGaugeInfo.entries.clear();
			breakGaugeInfo.isStunned = true;

			// state遷移をリクエスト
			stateManager_->RequestState(GameCharacterState::BREAK_STUN);
		}
	}

	// stepBoostのインターバルの更新
	enemyData_.characterInfo.stepBoostInfo.interval = 0.2f;
	if (enemyData_.characterInfo.stepBoostInfo.intervalTimer > 0.0f) {
		enemyData_.characterInfo.stepBoostInfo.intervalTimer -= deltaTime_;
	}

	// RUNNING時のState遷移リクエスト
	if (enemyData_.characterInfo.isInCombat) {
		// 1. AIスコアを更新（従来通り）
		float dist = (enemyData_.characterInfo.focusTargetPos -
			enemyData_.characterInfo.transform.translate).Length();
		aiBrainSystem_->SetIsBulletNearby(bulletSensor_->IsActive());
		aiBrainSystem_->SetDistanceToTarget(dist);
		aiBrainSystem_->Update();

		// 2. Blackboardにデータを集約
		UpdateBlackboard();

		// 3. ビヘイビアツリーを実行（ツリーがRequestStateを呼ぶ）
		if (behaviorTree_) {
			BehaviorStatus status = behaviorTree_->Execute(*blackboard_);
			if (status != BehaviorStatus::Running) {
				behaviorTree_->Reset(); // 完了 or 失敗 → 次フレームで再評価
			}
		}

		// 移動方向の取得（変更なし）
		enemyData_.characterInfo.moveDirection = inputProvider_->GetMoveDirection();

		// 攻撃処理（変更なし）
		if (enemyData_.characterInfo.isAlive == true &&
			(stateManager_->GetCurrentStateType() == GameCharacterState::BREAK_STUN) == false) {
			UpdateAttack();
		}
	}

	// エネルギーの更新
	UpdateEnergy();
	// アクティブブーストエフェクトのリクエスト
	RequestActiveBoostEffect();

	// Stateの更新
	stateManager_->Update(enemyData_.characterInfo);

	// 地面に着地したらRUNNINGに戻る
	if (enemyData_.characterInfo.onGround == true &&
		(stateManager_->GetCurrentStateType() == GameCharacterState::JUMP ||
			stateManager_->GetCurrentStateType() == GameCharacterState::FLOATING)) {

		stateManager_->RequestState(GameCharacterState::RUNNING);
	} else if (enemyData_.characterInfo.onGround == false &&
		stateManager_->GetCurrentStateType() ==GameCharacterState::RUNNING) {
		// 空中にいる場合はFLOATINGに切り替え
		stateManager_->RequestState(GameCharacterState::FLOATING);
	}

	// ダメージエフェクトの更新
	if (enemyData_.characterInfo.isDamaged) {
		damageEffectTime_ -= deltaTime_;
		particleEmitter_[0]->Emit();
		if (damageEffectTime_ <= 0.0f) {
			enemyData_.characterInfo.isDamaged = false;
		}
	}

	if (enemyData_.characterInfo.health <= 0.0f) {
		// 死亡状態のリクエスト
		enemyData_.characterInfo.isAlive = false;
		stateManager_->RequestState(State::DEAD);
		deadEffect_->Start();
	}

	// bulletSensorの更新
	bulletSensor_->SetTranslate(enemyData_.characterInfo.transform.translate);
	bulletSensor_->Update();

	auto jointWorldMatrixOpt =
		object3d_->GetModel()->GetSkeleton()->GetJointWorldMatrix(
			"body.002", object3d_->GetWorldMatrix());
	bodyPosition_ = { jointWorldMatrixOpt->m[3][0], jointWorldMatrixOpt->m[3][1],
					 jointWorldMatrixOpt->m[3][2] };

	// モデルの回転処理
	if (enemyData_.characterInfo.isAlive) {
		// 生存中はフォーカス対象に向く
		Quaternion targetRotate = QuaternionMath::LookRotation(
			Vector3Math::Normalize(enemyData_.characterInfo.focusTargetPos -
				enemyData_.characterInfo.transform.translate),
			{ 0.0f, 1.0f, 0.0f });

		enemyData_.characterInfo.transform.rotate = Easing::Slerp(
			enemyData_.characterInfo.transform.rotate, targetRotate, 0.1f);
		enemyData_.characterInfo.transform.rotate =
			QuaternionMath::Normalize(enemyData_.characterInfo.transform.rotate);

	} else {
		if (enemyData_.characterInfo.moveDirection.x != 0.0f ||
			enemyData_.characterInfo.moveDirection.z != 0.0f) {
			// 移動方向に合わせて回転
			float targetAngle = atan2(enemyData_.characterInfo.moveDirection.x,
				enemyData_.characterInfo.moveDirection.z);
			Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion(
				{ 0.0f, 1.0f, 0.0f }, targetAngle);
			enemyData_.characterInfo.transform.rotate = Easing::Slerp(
				enemyData_.characterInfo.transform.rotate, targetRotate, 0.5f);
			enemyData_.characterInfo.transform.rotate =
				QuaternionMath::Normalize(enemyData_.characterInfo.transform.rotate);
		}
	}

	// Quaternionからオイラー角に変換
	Vector3 eulerRotate =
		QuaternionMath::ToEuler(enemyData_.characterInfo.transform.rotate);
	object3d_->SetTranslate(enemyData_.characterInfo.transform.translate);
	object3d_->SetRotate(eulerRotate);
	// アニメーションコントローラの更新（object3d_->Update()より前に呼ぶ）
	animatorController_.Update(deltaTime_);
	object3d_->Update();
	object3d_->GetModel()->UpdateSkinningFromSkeleton();
	collider_->Update(object3d_.get());

	// 武器の更新
	for (auto& weapon : weapons_) {
		if (weapon) {
			weapon->SetTarget(enemyData_.characterInfo.focusTargetPos);
			weapon->SetTargetVelocity(focusTargetVelocity_);
			weapon->Update();
		}
	}

	// ブーストエフェクトの更新
	for (const auto& boostEffect : boostEffects_) {
		boostEffect->Update();
		boostEffect->SetCharacterState(stateManager_->GetCurrentStateType());
	}

	// パーティクルエミッターの更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(enemyData_.characterInfo.transform.translate);
		emitter->Update();
	}

	// 死亡エフェクトの更新
	deadEffect_->Update(bodyPosition_);

	// 着地判定の毎フレームリセット
	enemyData_.characterInfo.onGround = false;
}

void Enemy::UpdateImGui() {

#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::Begin("Enemy");

	if (blackboard_) {
		blackboard_->UpdateImGui();
	}

	ImGui::DragFloat3("Translate",
		&enemyData_.characterInfo.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &enemyData_.characterInfo.transform.scale.x,
		0.01f);
	ImGui::DragFloat4("Rotate", &enemyData_.characterInfo.transform.rotate.x,
		0.01f);
	ImGui::Separator();
	//health
	ImGui::DragFloat("Health", &enemyData_.characterInfo.health, 1.0f, 0.0f,
		enemyData_.characterInfo.maxHealth);
	//energy
	ImGui::DragFloat("Energy", &enemyData_.characterInfo.energyInfo.energy, 1.0f,
		0.0f, enemyData_.characterInfo.energyInfo.maxEnergy);
	//breakGauge
	ImGui::DragFloat("BreakGauge", &enemyData_.characterInfo.breakGaugeInfo.breakGauge,
		1.0f, 0.0f, enemyData_.characterInfo.breakGaugeInfo.maxBreakGauge);
	ImGui::DragFloat3("Velocity", &enemyData_.characterInfo.velocity.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &enemyData_.characterInfo.moveDirection.x,
		0.01f);
	ImGui::SeparatorText("Collider Info");
	ImGui::DragFloat3("offset", &enemyData_.characterInfo.colliderInfo.offset.x,
		0.01f);
	ImGui::DragFloat3("halfSize",
		&enemyData_.characterInfo.colliderInfo.halfSize.x, 0.01f);
	ImGui::Checkbox("OnGround", &enemyData_.characterInfo.onGround);

	// ダメージ処理テスト用ボタン
	if (ImGui::Button("Health Damage 1000")) {
		enemyData_.characterInfo.health -= 1000.0f;
		enemyData_.characterInfo.isDamaged = true;
		damageEffectTime_ = 0.5f;
		particleEmitter_[1]->Emit();
	}
	if (ImGui::Button("BreakStun Damage 100")) {
		BreakGaugeInfo& gaugeInfo = enemyData_.characterInfo.breakGaugeInfo;
		// ダメージに係数を乗算してゲージに加算（係数は武器ごとに変えてもよい）
		gaugeInfo.breakGauge += 100.0f;

		// 被弾履歴(遅延減衰待ち用)を追加
		BreakGaugeEntry entry;
		entry.amount = 100.0f; // ゲージに加算した値を保存
		entry.delayTimer.Initialize(entry.decayDelay, 0.0f); //減衰開始タイマーを初期化して開始
		gaugeInfo.entries.push_back(entry);

		

	}
	// スピードを0にするボタン
	if (ImGui::Button("Stop Movement")) {
		enemyData_.characterInfo.velocity = { 0.0f, 0.0f, 0.0f };
		enemyData_.characterInfo.jumpInfo.speed = 0.0f;
		enemyData_.characterInfo.moveSpeed = 0.0f;
		enemyData_.characterInfo.stepBoostInfo.speed = 0.0f;
	}

	// データ保存ボタン
	if (ImGui::Button("Save Enemy Data")) {
		SaveEnemyData(enemyData_.characterInfo.characterName);
	}
	ImGui::Separator();
	bulletSensor_->UpdateImGui();
	stateManager_->UpdateImGui();
	aiBrainSystem_->UpdateImGui();

	// コライダーの情報をEnemyDataから反映
	collider_->SetOffset(enemyData_.characterInfo.colliderInfo.offset);
	collider_->SetHalfSize(enemyData_.characterInfo.colliderInfo.halfSize);
	collider_->UpdateImGui("Enemy");
	for (auto& weapon : weapons_) {
		if (weapon) {
			weapon->UpdateImGui();
		}
	}

	ImGui::End();
#endif // _DEBUG
}

//========================================================================================================
// 描画処理
//========================================================================================================

void Enemy::Draw() {

	object3d_->Draw();
	for (const auto& weapon : weapons_) {
		if (weapon) {
			weapon->Draw();
		}
	}
}

void Enemy::DrawShadow(const LightCameraInfo& lightCamera) {
	object3d_->DrawShadow(lightCamera);
	for (const auto& weapon : weapons_) {
		weapon->DrawShadow(lightCamera);
	}
}

void Enemy::DrawCollider() {

#if defined(_DEBUG) || defined(_DEVELOP)
	collider_->DrawCollider();
	bulletSensor_->DrawCollider();
	// object3d_->GetModel()->GetSkeleton()->Draw(object3d_->GetWorldMatrix());
#endif // _DEBUG
}

//========================================================================================================
// 当たり判定の処理
//========================================================================================================

void Enemy::OnCollisionAction(GameCharacter* other) {

	if (other->GetCharacterType() == CharacterType::PLAYER) {
	}

	if (other->GetCharacterType() == CharacterType::PLAYER_BULLET) {

		Bullet* bullet = dynamic_cast<Bullet*>(other);
		// プレイヤーの弾に当たった場合の処理
		particleEmitter_[1]->Emit();
		enemyData_.characterInfo.isDamaged = true;
		// ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		// 体力を減らす
		enemyData_.characterInfo.health -= bullet->GetDamage();
		// ブレイクゲージを蓄積
		AccumulateBreakGauge(bullet->GetDamage());
	}
	if (other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
		// プレイヤーのミサイルに当たった場合の処理
		particleEmitter_[1]->Emit();
		enemyData_.characterInfo.isDamaged = true;
		// ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		// 体力を減らす
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		enemyData_.characterInfo.health -= missile->GetDamage();
		// ブレイクゲージを蓄積
		AccumulateBreakGauge(missile->GetDamage());
	}

	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		// playerのColliderが接触している面で判断する
		if (collider_->GetSurfaceType() == SurfaceType::BOTTOM) {

			// playerが下面で接触している場合は地面にいると判定
			enemyData_.characterInfo.onGround = true;
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				// 貫通している分だけ押し戻す
				// 過剰な押し戻しを防ぐために、penetrationDepthを2で割ると良い感じになる
				enemyData_.characterInfo.transform.translate +=
					-normal * penetrationDepth * 0.5f;

				// 接触面方向の速度を打ち消す
				float velocityAlongNormal =
					Vector3Math::Dot(enemyData_.characterInfo.velocity, normal);
				enemyData_.characterInfo.velocity -= normal * velocityAlongNormal;
			}

			collider_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // コライダーの色を緑に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::WALL) {

			// 側面で接触した場合
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				// 貫通している分だけ押し戻す
				enemyData_.characterInfo.transform.translate +=
					-normal * penetrationDepth;

				// 接触面方向の速度を打ち消す
				float dot = Vector3Math::Dot(enemyData_.characterInfo.velocity, normal);
				enemyData_.characterInfo.velocity -= normal * dot;
			}

			collider_->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f }); // コライダーの色を青に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::TOP) {
			// 天井に接触した場合
			if (enemyData_.characterInfo.velocity.y > 0.0f) {
				enemyData_.characterInfo.velocity.y = 0.0f;
			}

			collider_->SetColor(
				{ 1.0f, 1.0f, 0.0f, 1.0f }); // コライダーの色を黄色に変更
		}
	}
}

//===================================================================================
// 攻撃処理
//===================================================================================

void Enemy::UpdateAttack() {

	// AIにどの武器を使うか選ばせる
	std::vector<int> chosenWeapons = aiBrainSystem_->ChooseWeaponUnit(weapons_);

	// 選ばれた武器すべてに対して攻撃処理を行う
	for (int weaponIndex : chosenWeapons) {
		WeaponAttack(weaponIndex);
	}

	// チャージ攻撃可能なユニットの処理
	for (int i = 0; i < chargeShootableUnits_.size(); i++) {
		if (chargeShootableUnits_[i] == true) {
			// チャージ撃ち可能なユニットの処理
			auto* weapon = weapons_[i].get();
			if (enemyData_.characterInfo.isChargeShooting == true) {
				// チャージ撃ち中の処理
				chargeShootTimer_.Update();
				// イベントを発行して危険度の高い攻撃を行うことを知らせる

				if (chargeShootTimer_.IsFinished()) {
					weapon->Attack();
					enemyData_.characterInfo.isChargeShooting =
						false; // チャージ撃ち中フラグをリセット
					chargeShootTimer_.Stop();
					stateManager_->RequestState(GameCharacterState::CHARGESHOOT_STUN);
					chargeShootableUnits_[i] =
						false; // チャージ撃ち可能なユニットのマークをリセット
				}
			}
		}
	}
}

void Enemy::WeaponAttack(int weaponIndex) {

	auto* weapon = weapons_[weaponIndex].get();

	// チャージ攻撃可能な場合
	if (weapon->CanChargeAttack()) {

		// 武器のチャージ処理
		weapon->Charge(deltaTime_);
		if (ShouldReleaseAttack(weaponIndex)) {
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
			enemyData_.characterInfo.isChargeShooting =
				true; // チャージ撃ち中フラグを立てる
			chargeShootableUnits_[weaponIndex] =
				true; // チャージ撃ち可能なユニットとしてマーク

			// チャージ撃ちのタイマーを初期化して開始
			if (chargeShootTimer_.IsFinished()) {
				chargeShootTimer_.Initialize(chargeShootDuration_, 0.0f);
				// イベントを発行して危険度の高い攻撃を行うことを知らせる
				TakeCFrameWork::GetEventManager()->PostEvent(
					"EnemyHighPowerAttack",
					enemyData_.characterInfo.transform.translate);
			}

		} else {
			// 移動撃ち可能
			weapon->Attack();
		}
	}
	if (ShouldReleaseAttack(weaponIndex)) {
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
// 攻撃開始判定
//===================================================================================

bool Enemy::ShouldReleaseAttack(int weaponIndex) {
	auto* weapon = weapons_[weaponIndex].get();
	return weapon->GetChargeTime() >= weapon->GetRequiredChargeTime();
}

//===================================================================================
// エネルギーの更新処理
//===================================================================================

void Enemy::UpdateEnergy() {

	float& overheatTimer = enemyData_.characterInfo.overHeatInfo
		.overheatTimer; // オーバーヒートタイマー
	float maxEnergy =
		enemyData_.characterInfo.energyInfo.maxEnergy; // 最大エネルギー

	// オーバーヒートしているかどうか
	if (!enemyData_.characterInfo.overHeatInfo.isOverheated) {

		if (enemyData_.characterInfo.energyInfo.energy <= 0.0f) {
			// エネルギーが0以下になったらオーバーヒート状態にする
			enemyData_.characterInfo.overHeatInfo.isOverheated = true;
			overheatTimer = enemyData_.characterInfo.overHeatInfo
				.overheatDuration; // オーバーヒートのタイマーを設定
			enemyData_.characterInfo.energyInfo.energy = 0.0f; // エネルギーを0にする
		}

		// エネルギーの回復
		if (enemyData_.characterInfo.energyInfo.energy < maxEnergy) {

			// 浮遊状態,ジャンプ時、ステップブースト時はエネルギーを回復しない
			if (stateManager_->GetCurrentStateType() ==
				GameCharacterState::FLOATING ||
				stateManager_->GetCurrentStateType() == GameCharacterState::JUMP ||
				stateManager_->GetCurrentStateType() ==
				GameCharacterState::STEPBOOST) {
				return; // エネルギーの回復を行わない
			}

			enemyData_.characterInfo.energyInfo.energy +=
				enemyData_.characterInfo.energyInfo.recoveryRate * deltaTime_;
			if (enemyData_.characterInfo.energyInfo.energy > maxEnergy) {
				enemyData_.characterInfo.energyInfo.energy = maxEnergy;
			}
		}

	} else {

		// オーバーヒートの処理
		if (overheatTimer > 0.0f) {
			overheatTimer -= deltaTime_;
			if (overheatTimer <= 0.0f) {
				// オーバーヒートが終了したらエネルギーを半分回復
				enemyData_.characterInfo.energyInfo.energy = maxEnergy / 2.0f;
				// オーバーヒート状態を解除
				enemyData_.characterInfo.overHeatInfo.isOverheated = false;
			}
		}
	}
}

//===================================================================================
// ブーストエフェクトのアクティブ化判定
//===================================================================================

void Enemy::RequestActiveBoostEffect() {

	// ステップブーストの方向とスティックの向きによってアクティブにするエフェクトを変更
	Vector3 moveDir = enemyData_.characterInfo.moveDirection;

	if (moveDir.Length() <= 0.1f) {
		// スティックがほぼニュートラルの場合はすべてのエフェクトを非アクティブにして終了
		for (const auto& boostEffect : boostEffects_) {
			boostEffect->SetIsActive(false);
		}
		return;
	} else {
		// 背中のエフェクトは常にアクティブ
		boostEffects_[BACKPACK]->SetIsActive(true);
	}

	// --- プレイヤーの向きをQuaternionから取得 ---
	Vector3 localForward = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 playerForward = QuaternionMath::RotateVector(
		localForward, enemyData_.characterInfo.transform.rotate);
	playerForward.y = 0.0f; // 水平方向だけ見る
	playerForward = Vector3Math::Normalize(playerForward);

	// --- スティック方向との角度差を求める ---
	float dot = Vector3Math::Dot(playerForward, moveDir);
	float crossY = playerForward.x * moveDir.z - playerForward.z * moveDir.x;
	float angle =
		atan2(crossY, dot) * (180.0f / std::numbers::pi_v<float>); // -180°～180°

	// --- 角度差に応じてエフェクトをアクティブにする ---
	if (angle <= -45.0f && angle > -135.0f) {
		// 左
		boostEffects_[LEFT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_LEG]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(true);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(false);
	} else if (angle >= 45.0f && angle < 135.0f) {
		// 右
		boostEffects_[LEFT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_LEG]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(false);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(true);
	} else {
		// 前後
		boostEffects_[LEFT_LEG]->SetIsActive(true);
		boostEffects_[RIGHT_LEG]->SetIsActive(true);
		boostEffects_[LEFT_SHOULDER]->SetIsActive(false);
		boostEffects_[RIGHT_SHOULDER]->SetIsActive(false);
	}
}

//===================================================================================
// ブレイクゲージの蓄積
//===================================================================================
void Enemy::AccumulateBreakGauge(float damage) {

	auto& gaugeInfo = enemyData_.characterInfo.breakGaugeInfo;

	if (!enemyData_.characterInfo.isAlive ||
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
	entry.delayTimer.Initialize(entry.decayDelay, 0.0f); //減衰開始タイマーを初期化して開始
	gaugeInfo.entries.push_back(entry);

	// ゲージが最大値を超えたらスタン状態に遷移
	if (gaugeInfo.breakGauge >= gaugeInfo.maxBreakGauge) {
		gaugeInfo.breakGauge = gaugeInfo.maxBreakGauge;
		stateManager_->RequestState(GameCharacterState::BREAK_STUN);
	}
}

void Enemy::UpdateBlackboard() {

	// --- PlayableCharacterInfo からの書き込み ---
	blackboard_->SetValue("energy",    enemyData_.characterInfo.energyInfo.energy);
	blackboard_->SetValue("maxEnergy", enemyData_.characterInfo.energyInfo.maxEnergy);
	blackboard_->SetValue("hp",        enemyData_.characterInfo.health);
	blackboard_->SetValue("maxHp",     enemyData_.characterInfo.maxHealth);
	blackboard_->SetValue("hpRatio",   enemyData_.characterInfo.health / enemyData_.characterInfo.maxHealth);
	blackboard_->SetValue("posY",      enemyData_.characterInfo.transform.translate.y);
	blackboard_->SetValue("onGround",   enemyData_.characterInfo.onGround);
	blackboard_->SetValue("isOverheated", enemyData_.characterInfo.overHeatInfo.isOverheated);
	blackboard_->SetValue("isAlive",    enemyData_.characterInfo.isAlive);
	blackboard_->SetValue("isInCombat", enemyData_.characterInfo.isInCombat);

	// --- AIBrainSystem からの書き込み ---
	float distance = (enemyData_.characterInfo.focusTargetPos -
		enemyData_.characterInfo.transform.translate).Length();
	blackboard_->SetValue("distance",     distance);
	blackboard_->SetValue("orbitRadius",  orbitRadius_);
	blackboard_->SetValue("isBulletNearby", bulletSensor_->IsActive());

	// --- 武器スコアの書き込み ---
	for (size_t i = 0; i < weapons_.size(); ++i) {
		std::string key = "attackScore" + std::to_string(i);
		blackboard_->SetValue(key, aiBrainSystem_->GetAttackScore(static_cast<int>(i)));
	}
}
