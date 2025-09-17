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
	particleEmitter_[0]->Initialize("EnemyEmitter0",{ {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, characterInfo_.transform.translate }, 5, 1.0f);
	particleEmitter_[0]->SetParticleName("DamageSpark");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, characterInfo_.transform.translate }, 10, 1.0f);
	particleEmitter_[1]->SetParticleName("CrossEffect");
	//emitter2
	particleEmitter_[2] = std::make_unique<ParticleEmitter>();
	particleEmitter_[2]->Initialize("EnemyEmitter2", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, characterInfo_.transform.translate }, 10, 1.0f);
	particleEmitter_[2]->SetParticleName("SparkExplosion");

	weapons_.resize(2); // 武器の数を2つに設定
	weaponTypes_.resize(2);
	weaponTypes_[R_ARMS] = WeaponType::WEAPON_TYPE_RIFLE; // 1つ目の武器はライフル
	weaponTypes_[L_ARMS] = WeaponType::WEAPON_TYPE_BAZOOKA; // 2つ目の武器はバズーカ
	chargeShootableUnits_.resize(weapons_.size());

	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	inputProvider_ = std::make_unique<EnemyInputProvider>(this);

#pragma region charcterInfo

	characterInfo_.deceleration = 1.1f; //減速率
	characterInfo_.moveSpeed = 200.0f; //移動速度
	characterInfo_.kMaxMoveSpeed = 120.0f; //移動速度の最大値
	characterInfo_.maxHealth = 10000.0f; // 最大体力
	characterInfo_.health = characterInfo_.maxHealth; // 初期体力
	characterInfo_.stepBoostInfo.speed = 230.0f; // ステップブーストの速度
	characterInfo_.stepBoostInfo.duration = 0.3f; // ステップブーストの持続時間
	characterInfo_.stepBoostInfo.useEnergy = 100.0f; // ステップブーストに必要なエネルギー
	characterInfo_.stepBoostInfo.interval = 0.2f; // ステップブーストのインターバル
	characterInfo_.jumpInfo.speed = 50.0f; // ジャンプの速度
	characterInfo_.jumpInfo.maxJumpTime = 0.3f; // ジャンプの最大時間
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
	if(characterInfo_.stepBoostInfo.intervalTimer > 0.0f) {
		characterInfo_.stepBoostInfo.intervalTimer -= deltaTime_;
	}

	// StepBoost入力判定を最初に追加
	if (behaviorManager_->GetCurrentBehaviorType() == Behavior::RUNNING) {

	
			//1秒に1度確率で判定
			if( randomEngine() % 100 < jumpProbability_) { // 1%の確率
				if(randomEngine() % 2 == 0) {
					//Jump遷移
					behaviorManager_->RequestBehavior(Behavior::JUMP);
				} else {
					//StepBoost遷移
					behaviorManager_->RequestBehavior(Behavior::STEPBOOST);
				}
			}
	}

	//エネルギーの更新
	UpdateEnergy();

	//移動方向の取得
	characterInfo_.moveDirection = inputProvider_->GetMoveDirection();
	//Behaviorの更新
	behaviorManager_->Update(characterInfo_);

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

	if(characterInfo_.health <= 0.0f) {
		//死亡状態のリクエスト
		characterInfo_.isAlive = false;
		behaviorManager_->RequestBehavior(Behavior::DEAD);
	}

	characterInfo_.onGround = false; // 毎フレームリセット

	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::toEuler(characterInfo_.transform.rotate);
	object3d_->SetTranslate(characterInfo_.transform.translate);
	object3d_->SetRotate(eulerRotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	weapons_[0]->SetTarget(focusTargetPos_);
	weapons_[0]->Update();
	weapons_[1]->SetTarget(focusTargetPos_);
	weapons_[1]->Update(); 

	//パーティクルエミッターの更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(characterInfo_.transform.translate);
		emitter->Update();
	}
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DamageSpark")->SetEmitterPosition(characterInfo_.transform.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(characterInfo_.transform.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SparkExplosion")->SetEmitterPosition(characterInfo_.transform.translate);
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
	behaviorManager_->UpdateImGui();
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
		characterInfo_.isDamaged = true;
		//ダメージを受けた時のエフェクト時間を設定
		damageEffectTime_ = 0.5f;
		//体力を減らす
		characterInfo_.health -= bullet->GetDamage();
	}
	if(other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
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
				characterInfo_.transform.translate += -normal * penetrationDepth;

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

	WeaponAttack(R_ARMS); // 右手の武器で攻撃
	WeaponAttack(L_ARMS); // 左手の武器で攻撃

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
	if (ShouldStartAttack(weaponIndex)) {
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
	} else if (ShouldReleaseAttack(weaponIndex)) {
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

bool Enemy::ShouldStartAttack(int weaponIndex) {
	// 例: ターゲットとの距離が射程範囲でクールタイムが終わってたら攻撃
	auto* weapon = weapons_[weaponIndex].get();
	float distance = (focusTargetPos_ - characterInfo_.transform.translate).Length();
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