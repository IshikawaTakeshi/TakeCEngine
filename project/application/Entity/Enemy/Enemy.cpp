#include "Enemy.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Model.h"
#include "engine/Input/Input.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/Utility/JsonLoader.h"
#include "Vector3Math.h"
#include "math/Easing.h"

#include "application/Weapon/Rifle/Rifle.h"
#include "application/Weapon/Bazooka/Bazooka.h"
#include "application/Weapon/Launcher/VerticalMissileLauncher.h"
#include "application/Weapon/MachineGun/MachineGun.h"
#include "application/Weapon/ShotGun/ShotGun.h"
#include "application/Entity/WeaponUnit.h"

//========================================================================================================
//　初期化
//========================================================================================================
void Enemy::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//キャラクタータイプ設定
	characterType_ = CharacterType::ENEMY;

	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetOwner(this);
	collider_->SetHalfSize({ 2.0f, 2.5f, 2.0f }); // コライダーの半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Enemy));

	enemyData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<CharacterData>("EnemyData.json");
	object3d_->SetScale(enemyData_.characterInfo.transform.scale);

	//emitter設定
	//emitter0
	particleEmitter_.resize(3);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0", "DamageSpark.json");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", "DamageSpark2.json");
	//emitter2
	particleEmitter_[2] = std::make_unique<ParticleEmitter>();
	particleEmitter_[2]->Initialize("EnemyEmitter2", "SparkExplosion.json");
	//背部エミッターの初期化
	backEmitter_ = std::make_unique<ParticleEmitter>();
	backEmitter_->Initialize("EnemyBackpack", "WalkSmoke2.json");

	//死亡エフェクト初期化
	deadEffect_ = std::make_unique<DeadEffect>();
	deadEffect_->Initialize();

	//武器の初期化
	weapons_.resize(WeaponUnit::Size);
	weaponTypes_.resize(WeaponUnit::Size);
	for(size_t i = 0; i < weaponTypes_.size(); i++) {
		weaponTypes_[i] = enemyData_.weaponData[i].weaponType;
	}
	chargeShootableUnits_.resize(weapons_.size());

	//デルタタイムの取得
	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	//bulletSensor_の初期化
	bulletSensor_ = std::make_unique<BulletSensor>();
	bulletSensor_->Initialize(object3dCommon, "ICOBall.gltf");
	bulletSensor_->SetSensorRadius(collider_->GetHalfSize().x * 20.0f);

	//AIBrainSystemの初期化
	aiBrainSystem_ = std::make_unique<AIBrainSystem>();
	aiBrainSystem_->Initialize(&enemyData_.characterInfo, weapons_.size());
	aiBrainSystem_->SetOrbitRadius(orbitRadius_);

	//InputProviderの初期化
	inputProvider_ = std::make_unique<EnemyInputProvider>(this);
	inputProvider_->SetAIBrainSystem(aiBrainSystem_.get());

	//BehaviorManagerの初期化
	behaviorManager_ = std::make_unique<BehaviorManager>();
	behaviorManager_->Initialize(inputProvider_.get());
	behaviorManager_->InitializeBehaviors(enemyData_.characterInfo);

	
}

//========================================================================================================
// 武器の初期化
//========================================================================================================
void Enemy::WeaponInitialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {
	//武器の初期化
	for (int i = 0; i < weapons_.size(); i++) {
		if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_RIFLE) {
			//ライフルの武器を初期化
			weapons_[i] = std::make_unique<Rifle>();
			
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_BAZOOKA) {
			//バズーカの武器を初期化
			weapons_[i] = std::make_unique<Bazooka>();
			
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_VERTICAL_MISSILE) {
			//垂直ミサイルの武器を初期化
			weapons_[i] = std::make_unique<VerticalMissileLauncher>();
			
		} else if(weaponTypes_[i] == WeaponType::WEAPON_TYPE_MACHINE_GUN) {
			//マシンガンの武器を初期化
			weapons_[i] = std::make_unique<MachineGun>();
			
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_SHOTGUN) {
			//ショットガンの武器を初期化
			weapons_[i] = std::make_unique<ShotGun>();
			
		} else {
			//武器が設定されていない場合はnullptrのまま
			weapons_[i] = nullptr;
		}

		weapons_[i]->Initialize(object3dCommon, bulletManager);
		weapons_[i]->SetOwnerObject(this);
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

//========================================================================================================
// EnemyのGameCharacterContextの読み込み・保存
//========================================================================================================
void Enemy::LoadEnemyData(const std::string& characterJsonPath) {

	//JsonLoaderを使ってEnemyのGameCharacterContextを読み込み
	enemyData_.characterInfo = TakeCFrameWork::GetJsonLoader()->LoadJsonData<PlayableCharacterInfo>(characterJsonPath);
}

void Enemy::SaveEnemyData(const std::string& characterJsonPath) {

	//JsonLoaderを使ってEnemyのGameCharacterContextを保存
	enemyData_.characterInfo.characterName = characterJsonPath;
	TakeCFrameWork::GetJsonLoader()->SaveJsonData<PlayableCharacterInfo>(characterJsonPath, enemyData_.characterInfo);
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Enemy::Update() {

	//stepBoostのインターバルの更新
	if (enemyData_.characterInfo.stepBoostInfo.intervalTimer > 0.0f) {
		enemyData_.characterInfo.stepBoostInfo.intervalTimer -= deltaTime_;
	}

	// RUNNING時のBehavior遷移リクエスト
	if (behaviorManager_->GetCurrentBehaviorType() == Behavior::RUNNING) {
		
		//BestActionの取得
		switch (aiBrainSystem_->GetBestAction()) {
		case Action::JUMP: 
			behaviorManager_->RequestBehavior(Behavior::JUMP);
			break;
		case Action::FLOATING: // FLOATINGへの遷移はJUMPで行う
			behaviorManager_->RequestBehavior(Behavior::JUMP);
			break;
		case Action::STEPBOOST:
			behaviorManager_->RequestBehavior(Behavior::STEPBOOST);
			break;	
		}
	}

	//エネルギーの更新
	UpdateEnergy();
	//移動方向の取得
	enemyData_.characterInfo.moveDirection = inputProvider_->GetMoveDirection();
	//Behaviorの更新
	behaviorManager_->Update(enemyData_.characterInfo);

	// 地面に着地したらRUNNINGに戻る
	if (enemyData_.characterInfo.onGround == true &&
		(behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP ||
			behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING)) {
		behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
	} else if (enemyData_.characterInfo.onGround == false &&
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING) {
		//空中にいる場合はFLOATINGに切り替え
		behaviorManager_->RequestBehavior(GameCharacterBehavior::FLOATING);
	}

	//攻撃処理
	if (enemyData_.characterInfo.isAlive == true) {
		UpdateAttack();
	}

	//ダメージエフェクトの更新
	if (enemyData_.characterInfo.isDamaged) {
		damageEffectTime_ -= deltaTime_;
		particleEmitter_[0]->Emit();
		if (damageEffectTime_ <= 0.0f) {
			enemyData_.characterInfo.isDamaged = false;
		}
	}

	if (enemyData_.characterInfo.health <= 0.0f) {
		//死亡状態のリクエスト
		enemyData_.characterInfo.isAlive = false;
		behaviorManager_->RequestBehavior(Behavior::DEAD);
		deadEffect_->Start();
	}

	//bulletSensorの更新
	bulletSensor_->SetTranslate(enemyData_.characterInfo.transform.translate);
	bulletSensor_->Update();

	//AIの更新
	float distance = (enemyData_.characterInfo.focusTargetPos - enemyData_.characterInfo.transform.translate).Length();
	aiBrainSystem_->SetIsBulletNearby(bulletSensor_->IsActive());
	aiBrainSystem_->SetDistanceToTarget(distance);
	aiBrainSystem_->Update();

	if (enemyData_.characterInfo.onGround && (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING ||
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::STEPBOOST)) {
		backEmitter_->SetIsEmit(true);
	} else {
		backEmitter_->SetIsEmit(false);
	}

	auto jointWorldMatrixOpt = object3d_->GetModel()->GetSkeleton()->GetJointWorldMatrix("neck", object3d_->GetWorldMatrix());
	bodyPosition_ = {
		jointWorldMatrixOpt->m[3][0],
		jointWorldMatrixOpt->m[3][1],
		jointWorldMatrixOpt->m[3][2]
	};

	//モデルの回転処理
	if (enemyData_.characterInfo.isAlive) {
		//生存中はフォーカス対象に向く
		Quaternion targetRotate = QuaternionMath::LookRotation(
			Vector3Math::Normalize(enemyData_.characterInfo.focusTargetPos - enemyData_.characterInfo.transform.translate),
			{ 0.0f,1.0f,0.0f });
		
		enemyData_.characterInfo.transform.rotate = Easing::Slerp(enemyData_.characterInfo.transform.rotate, targetRotate, 0.1f);
		enemyData_.characterInfo.transform.rotate = QuaternionMath::Normalize(enemyData_.characterInfo.transform.rotate);

	} else {
		if (enemyData_.characterInfo.moveDirection.x != 0.0f || enemyData_.characterInfo.moveDirection.z != 0.0f) {
			//移動方向に合わせて回転
			float targetAngle = atan2(enemyData_.characterInfo.moveDirection.x, enemyData_.characterInfo.moveDirection.z);
			Quaternion targetRotate = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.0f,1.0f,0.0f }, targetAngle);
			enemyData_.characterInfo.transform.rotate = Easing::Slerp(enemyData_.characterInfo.transform.rotate, targetRotate, 0.05f);
			enemyData_.characterInfo.transform.rotate = QuaternionMath::Normalize(enemyData_.characterInfo.transform.rotate);
		}
	}
	
	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::ToEuler(enemyData_.characterInfo.transform.rotate);
	object3d_->SetTranslate(enemyData_.characterInfo.transform.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//武器の更新
	for(auto& weapon : weapons_) {
		if(weapon) {
			weapon->SetTarget(enemyData_.characterInfo.focusTargetPos);
			weapon->SetTargetVelocity(focusTargetVelocity_);
			weapon->Update();
		}
	}

	//パーティクルエミッターの更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(enemyData_.characterInfo.transform.translate);
		emitter->Update();
	}

	//背部エミッターの更新
	std::optional<Vector3> backpackPosition = object3d_->GetModel()->GetSkeleton()->GetJointPosition("leg", object3d_->GetWorldMatrix());
	backEmitter_->SetTranslate(backpackPosition.value());
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("WalkSmoke2")->SetEmitterPosition(backpackPosition.value());
	backEmitter_->Update();
	//死亡エフェクトの更新
	deadEffect_->Update(enemyData_.characterInfo.transform.translate);

	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DamageSpark")->SetEmitterPosition(enemyData_.characterInfo.transform.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(enemyData_.characterInfo.transform.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SparkExplosion")->SetEmitterPosition(enemyData_.characterInfo.transform.translate);

	// 着地判定の毎フレームリセット
	enemyData_.characterInfo.onGround = false; 

}

void Enemy::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("Translate", &enemyData_.characterInfo.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &enemyData_.characterInfo.transform.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &enemyData_.characterInfo.transform.rotate.x, 0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &enemyData_.characterInfo.health, 1.0f, 0.0f, enemyData_.characterInfo.maxHealth);
	ImGui::DragFloat("Energy", &enemyData_.characterInfo.energyInfo.energy, 1.0f, 0.0f, enemyData_.characterInfo.energyInfo.maxEnergy);
	ImGui::DragFloat3("Velocity", &enemyData_.characterInfo.velocity.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &enemyData_.characterInfo.moveDirection.x, 0.01f);
	ImGui::Checkbox("OnGround", &enemyData_.characterInfo.onGround);

	// ダメージ処理テスト用ボタン
	if(ImGui::Button("Damage 1000")) {
		enemyData_.characterInfo.health -= 1000.0f;
		enemyData_.characterInfo.isDamaged = true;
		damageEffectTime_ = 0.5f;
		particleEmitter_[1]->Emit();
	}
	// スピードを0にするボタン
	if (ImGui::Button("Stop Movement")) {
		enemyData_.characterInfo.velocity = { 0.0f,0.0f,0.0f };
		enemyData_.characterInfo.jumpInfo.speed = 0.0f;
		enemyData_.characterInfo.moveSpeed = 0.0f;
		enemyData_.characterInfo.stepBoostInfo.speed = 0.0f;
	}

	//データ保存ボタン
	if (ImGui::Button("Save Enemy Data")) {
		SaveEnemyData(enemyData_.characterInfo.characterName);
	}
	ImGui::Separator();
	bulletSensor_->UpdateImGui();
	behaviorManager_->UpdateImGui();
	aiBrainSystem_->UpdateImGui();
	collider_->UpdateImGui("Enemy");
	for(auto& weapon : weapons_) {
		if(weapon) {
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
		weapon->Draw();
	}

}

void Enemy::DrawShadow(const LightCameraInfo& lightCamera) {
	object3d_->DrawShadow(lightCamera);
	for (const auto& weapon : weapons_) {
		weapon->DrawShadow(lightCamera);
	}
}

void Enemy::DrawCollider() {

#ifdef _DEBUG
	//collider_->DrawCollider();
	bulletSensor_->DrawCollider();

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
		//プレイヤーの弾に当たった場合の処理
		particleEmitter_[1]->Emit();
		enemyData_.characterInfo.isDamaged = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		enemyData_.characterInfo.health -= bullet->GetDamage();
	}
	if (other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
		//プレイヤーのミサイルに当たった場合の処理
		particleEmitter_[1]->Emit();
		enemyData_.characterInfo.isDamaged = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		enemyData_.characterInfo.health -= missile->GetDamage();
	}


	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//playerのColliderが接触している面で判断する
		if (collider_->GetSurfaceType() == SurfaceType::BOTTOM) {

			//playerが下面で接触している場合は地面にいると判定
			enemyData_.characterInfo.onGround = true;
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				//貫通している分だけ押し戻す
				//過剰な押し戻しを防ぐために、penetrationDepthを2で割ると良い感じになる
				enemyData_.characterInfo.transform.translate += -normal * penetrationDepth * 0.5f;

				//接触面方向の速度を打ち消す
				float velocityAlongNormal = Vector3Math::Dot(enemyData_.characterInfo.velocity, normal);
				enemyData_.characterInfo.velocity -= normal * velocityAlongNormal;

			}


			collider_->SetColor({ 0.0f,1.0f,0.0f,1.0f }); // コライダーの色を緑に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::WALL) {

			//側面で接触した場合
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				//貫通している分だけ押し戻す
				enemyData_.characterInfo.transform.translate += -normal * penetrationDepth;

				//接触面方向の速度を打ち消す
				float dot = Vector3Math::Dot(enemyData_.characterInfo.velocity, normal);
				enemyData_.characterInfo.velocity -= normal * dot;

			}

			collider_->SetColor({ 0.0f,0.0f,1.0f,1.0f }); // コライダーの色を青に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::TOP) {
			//天井に接触した場合
			if (enemyData_.characterInfo.velocity.y > 0.0f) {
				enemyData_.characterInfo.velocity.y = 0.0f;
			}

			collider_->SetColor({ 1.0f,1.0f,0.0f,1.0f }); // コライダーの色を黄色に変更
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

	//チャージ攻撃可能なユニットの処理
	for (int i = 0; i < chargeShootableUnits_.size(); i++) {
		if (chargeShootableUnits_[i] == true) {
			//チャージ撃ち可能なユニットの処理
			auto* weapon = weapons_[i].get();
			if (enemyData_.characterInfo.isChargeShooting == true) {
				// チャージ撃ち中の処理
				chargeShootTimer_ -= deltaTime_;
				if (chargeShootTimer_ <= 0.0f) {
					weapon->Attack();
					enemyData_.characterInfo.isChargeShooting = false; // チャージ撃ち中フラグをリセット
					chargeShootTimer_ = 0.0f; // タイマーをリセット
					behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
					chargeShootableUnits_[i] = false; // チャージ撃ち可能なユニットのマークをリセット
				}
			}
		}
	}
}

void Enemy::WeaponAttack(int weaponIndex) {

	auto* weapon = weapons_[weaponIndex].get();

	//チャージ攻撃可能な場合
	if (weapon->CanChargeAttack()) {

		//武器のチャージ処理
		weapon->Charge(deltaTime_);
		if (ShouldReleaseAttack(weaponIndex)) {
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
			enemyData_.characterInfo.isChargeShooting = true; // チャージ撃ち中フラグを立てる
			chargeShootTimer_ = chargeShootDuration_; // チャージ撃ちのタイマーを設定
			chargeShootableUnits_[weaponIndex] = true; // チャージ撃ち可能なユニットとしてマーク

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
				behaviorManager_->RequestBehavior(GameCharacterBehavior::CHARGESHOOT_STUN);
			} else {
				// 移動撃ち可能な場合はRUNNINGに戻す
				behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
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

	float& overheatTimer = enemyData_.characterInfo.overHeatInfo.overheatTimer; // オーバーヒートタイマー
	float maxEnergy = enemyData_.characterInfo.energyInfo.maxEnergy; // 最大エネルギー

	//オーバーヒートしているかどうか
	if (!enemyData_.characterInfo.overHeatInfo.isOverheated) {

		if (enemyData_.characterInfo.energyInfo.energy <= 0.0f) {
			// エネルギーが0以下になったらオーバーヒート状態にする
			enemyData_.characterInfo.overHeatInfo.isOverheated = true;
			overheatTimer = enemyData_.characterInfo.overHeatInfo.overheatDuration; // オーバーヒートのタイマーを設定
			enemyData_.characterInfo.energyInfo.energy = 0.0f; // エネルギーを0にする
		}

		// エネルギーの回復
		if (enemyData_.characterInfo.energyInfo.energy < maxEnergy) {

			//浮遊状態,ジャンプ時、ステップブースト時はエネルギーを回復しない
			if (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING ||
				behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP ||
				behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::STEPBOOST) {
				return; // エネルギーの回復を行わない
			}

			enemyData_.characterInfo.energyInfo.energy += enemyData_.characterInfo.energyInfo.recoveryRate * deltaTime_;
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