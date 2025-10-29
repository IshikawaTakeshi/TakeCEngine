#include "Enemy.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Model.h"
#include "engine/io/Input.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/Utility/JsonLoader.h"
#include "Vector3Math.h"
#include "math/Easing.h"

#include "application/Weapon/Rifle.h"
#include "application/Weapon/Bazooka.h"
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
	collider_->SetHalfSize({ 2.0f, 2.5f, 2.0f }); // コライダーの半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Enemy));

	characterInfo_.transform = { {1.5f,1.5f,1.5f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,30.0f} };
	object3d_->SetScale(characterInfo_.transform.scale);

	//emiiter設定
	//emitter0
	particleEmitter_.resize(3);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, characterInfo_.transform.translate }, 5, 1.0f);
	particleEmitter_[0]->SetParticleName("DamageSpark");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, characterInfo_.transform.translate }, 10, 1.0f);
	particleEmitter_[1]->SetParticleName("CrossEffect");
	//emitter2
	particleEmitter_[2] = std::make_unique<ParticleEmitter>();
	particleEmitter_[2]->Initialize("EnemyEmitter2", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, characterInfo_.transform.translate }, 10, 1.0f);
	particleEmitter_[2]->SetParticleName("SparkExplosion");
	//背部エミッターの初期化
	backEmitter_ = std::make_unique<ParticleEmitter>();
	backEmitter_->Initialize("EnemyBackpack", object3d_->GetTransform(), 10, 0.01f);
	backEmitter_->SetParticleName("WalkSmoke2");

	//死亡エフェクト初期化
	deadEffect_ = std::make_unique<DeadEffect>();
	deadEffect_->Initialize();

	weapons_.resize(2); // 武器の数を2つに設定
	weaponTypes_.resize(2);
	weaponTypes_[R_ARMS] = WeaponType::WEAPON_TYPE_RIFLE; // 1つ目の武器はライフル
	weaponTypes_[L_ARMS] = WeaponType::WEAPON_TYPE_BAZOOKA; // 2つ目の武器はバズーカ
	chargeShootableUnits_.resize(weapons_.size());

	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	//bulletSensor_の初期化
	bulletSensor_ = std::make_unique<BulletSensor>();
	bulletSensor_->Initialize(object3dCommon, "Sphere.gltf");
	bulletSensor_->SetSensorRadius(collider_->GetHalfSize().x * 20.0f);

	//AIBrainSystemの初期化
	aiBrainSystem_ = std::make_unique<AIBrainSystem>();
	aiBrainSystem_->Initialize(&characterInfo_, weapons_.size());
	aiBrainSystem_->SetOrbitRadius(orbitRadius_);

	//InputProviderの初期化
	inputProvider_ = std::make_unique<EnemyInputProvider>(this);
	inputProvider_->SetAIBrainSystem(aiBrainSystem_.get());

	//BehaviorManagerの初期化
	behaviorManager_ = std::make_unique<BehaviorManager>();
	behaviorManager_->Initialize(inputProvider_.get());
	behaviorManager_->InitializeBehaviors(characterInfo_);
}

//========================================================================================================
// 武器の初期化
//========================================================================================================
void Enemy::WeaponInitialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {
	//武器の初期化
	for (int i = 0; i < weapons_.size(); i++) {
		if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_RIFLE) {
			weapons_[i] = std::make_unique<Rifle>();
			weapons_[i]->Initialize(object3dCommon, bulletManager, "Rifle.gltf");
			weapons_[i]->SetOwnerObject(this);
		} else if (weaponTypes_[i] == WeaponType::WEAPON_TYPE_BAZOOKA) {
			weapons_[i] = std::make_unique<Bazooka>();
			weapons_[i]->Initialize(object3dCommon, bulletManager, "Bazooka.gltf");
			weapons_[i]->SetOwnerObject(this);
		}
	}

	weapons_[R_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "RightHand"); // 1つ目の武器を右手に取り付け
	weapons_[L_ARMS]->AttachToSkeletonJoint(object3d_->GetModel()->GetSkeleton(), "LeftHand"); // 2つ目の武器を左手に取り付け
}

//========================================================================================================
// EnemyのGameCharacterContextの読み込み・保存
//========================================================================================================
void Enemy::LoadEnemyData(const std::string& characterName) {

	//JsonLoaderを使ってEnemyのGameCharacterContextを読み込み
	characterInfo_ = TakeCFrameWork::GetJsonLoader()->LoadGameCharacterContext(characterName);
}

void Enemy::SaveEnemyData(const std::string& characterName) {

	//JsonLoaderを使ってEnemyのGameCharacterContextを保存
	characterInfo_.name = characterName;
	TakeCFrameWork::GetJsonLoader()->SaveGameCharacterContext(characterName, characterInfo_);
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Enemy::Update() {

	//stepBoostのインターバルの更新
	if (characterInfo_.stepBoostInfo.intervalTimer > 0.0f) {
		characterInfo_.stepBoostInfo.intervalTimer -= deltaTime_;
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
	characterInfo_.moveDirection = inputProvider_->GetMoveDirection();
	//Behaviorの更新
	behaviorManager_->Update(characterInfo_);

	// 地面に着地したらRUNNINGに戻る
	if (characterInfo_.onGround == true &&
		(behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::JUMP ||
			behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::FLOATING)) {
		behaviorManager_->RequestBehavior(GameCharacterBehavior::RUNNING);
	} else if (characterInfo_.onGround == false &&
		behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING) {
		//空中にいる場合はFLOATINGに切り替え
		behaviorManager_->RequestBehavior(GameCharacterBehavior::FLOATING);
	}

	//攻撃処理
	if (characterInfo_.isAlive == true) {
		UpdateAttack();
	}

	//ダメージエフェクトの更新
	if (characterInfo_.isDamaged) {
		damageEffectTime_ -= deltaTime_;
		particleEmitter_[0]->Emit();
		if (damageEffectTime_ <= 0.0f) {
			characterInfo_.isDamaged = false;
		}
	}

	if (characterInfo_.health <= 0.0f) {
		//死亡状態のリクエスト
		characterInfo_.isAlive = false;
		behaviorManager_->RequestBehavior(Behavior::DEAD);
		deadEffect_->Start();
	}

	//bulletSensorの更新
	bulletSensor_->SetTranslate(characterInfo_.transform.translate);
	bulletSensor_->Update();

	//AIの更新
	float distance = (characterInfo_.focusTargetPos - characterInfo_.transform.translate).Length();
	aiBrainSystem_->SetIsBulletNearby(bulletSensor_->IsActive());
	aiBrainSystem_->SetDistanceToTarget(distance);
	aiBrainSystem_->Update();

	if (characterInfo_.onGround && (behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::RUNNING || behaviorManager_->GetCurrentBehaviorType() == GameCharacterBehavior::STEPBOOST)) {
		backEmitter_->SetIsEmit(true);
	} else {
		backEmitter_->SetIsEmit(false);
	}
	
	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::ToEuler(characterInfo_.transform.rotate);
	object3d_->SetTranslate(characterInfo_.transform.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//武器の更新
	weapons_[0]->SetTarget(characterInfo_.focusTargetPos);
	weapons_[0]->SetTargetVelocity(focusTargetVelocity_);
	weapons_[0]->Update();
	weapons_[1]->SetTarget(characterInfo_.focusTargetPos);
	weapons_[1]->SetTargetVelocity(focusTargetVelocity_);
	weapons_[1]->Update();

	//パーティクルエミッターの更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(characterInfo_.transform.translate);
		emitter->Update();
	}

	//背部エミッターの更新
	std::optional<Vector3> backpackPosition = object3d_->GetModel()->GetSkeleton()->GetJointPosition("leg", object3d_->GetWorldMatrix());
	backEmitter_->SetTranslate(backpackPosition.value());
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("WalkSmoke2")->SetEmitterPosition(backpackPosition.value());
	backEmitter_->Update();
	//死亡エフェクトの更新
	deadEffect_->Update(characterInfo_.transform.translate);

	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DamageSpark")->SetEmitterPosition(characterInfo_.transform.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(characterInfo_.transform.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SparkExplosion")->SetEmitterPosition(characterInfo_.transform.translate);

	// 着地判定の毎フレームリセット
	characterInfo_.onGround = false; 

}

void Enemy::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("Translate", &characterInfo_.transform.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &characterInfo_.transform.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &characterInfo_.transform.rotate.x, 0.01f);
	ImGui::Separator();
	ImGui::DragFloat("Health", &characterInfo_.health, 1.0f, 0.0f, characterInfo_.maxHealth);
	ImGui::DragFloat("Energy", &characterInfo_.energyInfo.energy, 1.0f, 0.0f, characterInfo_.energyInfo.maxEnergy);
	ImGui::DragFloat3("Velocity", &characterInfo_.velocity.x, 0.01f);
	ImGui::DragFloat3("MoveDirection", &characterInfo_.moveDirection.x, 0.01f);
	ImGui::Checkbox("OnGround", &characterInfo_.onGround);

	// ダメージ処理テスト用ボタン
	if(ImGui::Button("Damage 1000")) {
		characterInfo_.health -= 1000.0f;
		characterInfo_.isDamaged = true;
		damageEffectTime_ = 0.5f;
		particleEmitter_[1]->Emit();
	}
	//データ保存ボタン
	if (ImGui::Button("Save Enemy Data")) {
		SaveEnemyData(characterInfo_.name);
	}
	ImGui::Separator();
	bulletSensor_->UpdateImGui();
	behaviorManager_->UpdateImGui();
	aiBrainSystem_->UpdateImGui();
	collider_->UpdateImGui("Enemy");
	weapons_[0]->UpdateImGui();
	weapons_[1]->UpdateImGui();
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
		characterInfo_.isDamaged = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		characterInfo_.health -= bullet->GetDamage();
	}
	if (other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
		//プレイヤーのミサイルに当たった場合の処理
		//particleEmitter_[2]->Emit();
		characterInfo_.isDamaged = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		VerticalMissile* missile = dynamic_cast<VerticalMissile*>(other);
		characterInfo_.health -= missile->GetDamage();
	}


	if (other->GetCharacterType() == CharacterType::LEVEL_OBJECT) {

		//playerのColliderが接触している面で判断する
		if (collider_->GetSurfaceType() == SurfaceType::BOTTOM) {

			//playerが下面で接触している場合は地面にいると判定
			characterInfo_.onGround = true;
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				//貫通している分だけ押し戻す
				//過剰な押し戻しを防ぐために、penetrationDepthを2で割ると良い感じになる
				characterInfo_.transform.translate += -normal * penetrationDepth * 0.5f;

				//接触面方向の速度を打ち消す
				float velocityAlongNormal = Vector3Math::Dot(characterInfo_.velocity, normal);
				characterInfo_.velocity -= normal * velocityAlongNormal;

			}


			collider_->SetColor({ 0.0f,1.0f,0.0f,1.0f }); // コライダーの色を緑に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::WALL) {

			//側面で接触した場合
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider_.get())) {
				Vector3 normal = box->GetMinAxis();
				float penetrationDepth = box->GetMinPenetration();

				//貫通している分だけ押し戻す
				characterInfo_.transform.translate += -normal * penetrationDepth;

				//接触面方向の速度を打ち消す
				float dot = Vector3Math::Dot(characterInfo_.velocity, normal);
				characterInfo_.velocity -= normal * dot;

			}

			collider_->SetColor({ 0.0f,0.0f,1.0f,1.0f }); // コライダーの色を青に変更

		} else if (collider_->GetSurfaceType() == SurfaceType::TOP) {
			//天井に接触した場合
			if (characterInfo_.velocity.y > 0.0f) {
				characterInfo_.velocity.y = 0.0f;
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
			if (characterInfo_.isChargeShooting == true) {
				// チャージ撃ち中の処理
				chargeShootTimer_ -= deltaTime_;
				if (chargeShootTimer_ <= 0.0f) {
					weapon->Attack();
					characterInfo_.isChargeShooting = false; // チャージ撃ち中フラグをリセット
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
	if (weapon->IsChargeAttack()) {

		//武器のチャージ処理
		weapon->Charge(deltaTime_);
		if (ShouldReleaseAttack(weaponIndex)) {
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
			if (weapon->IsStopShootOnly()) {
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

	float& overheatTimer = characterInfo_.overHeatInfo.overheatTimer; // オーバーヒートタイマー
	float maxEnergy = characterInfo_.energyInfo.maxEnergy; // 最大エネルギー

	//オーバーヒートしているかどうか
	if (!characterInfo_.overHeatInfo.isOverheated) {

		if (characterInfo_.energyInfo.energy <= 0.0f) {
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
			overheatTimer -= deltaTime_;
			if (overheatTimer <= 0.0f) {
				// オーバーヒートが終了したらエネルギーを半分回復
				characterInfo_.energyInfo.energy = maxEnergy / 2.0f;
				// オーバーヒート状態を解除
				characterInfo_.overHeatInfo.isOverheated = false;
			}
		}

	}
}