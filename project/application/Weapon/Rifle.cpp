#include "Rifle.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

//===================================================================================
//　初期化処理
//===================================================================================
void Rifle::Initialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& filePath) {

	bulletManager_ = bulletManager;

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	// ライフルの色を設定
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.5f, 0.5f, 0.0f, 1.0f });
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetEnvCoefficient(0.8f);

	//武器の初期化
	weaponType_ = WeaponType::WEAPON_TYPE_RIFLE;
	damage_ = 90.0f;
	attackInterval_ = kAttackInterval;
	bulletSpeed_ = 550.0f; // 弾のスピードを設定
	magazineCount_ = 30; // マガジン内の弾数を設定
	bulletCount_ = magazineCount_;
	maxBulletCount_ = 600;
	remainingBulletCount_ = maxBulletCount_; // 残弾数を最大弾数に設定
	effectiveRange_ = 700.0f; // 有効射程距離を設定

	maxReloadTime_ = 3.0f; // 最大リロード時間を設定

	canChargeAttack_ = false; // ライフルはチャージ攻撃可能
	canMoveShootable_ = true; // ライフルは移動撃ち可能
	isStopShootOnly_ = false; // ライフルは停止撃ち専用ではない
}

//===================================================================================
//　更新処理
//===================================================================================
void Rifle::Update() {

	if(remainingBulletCount_ <= 0 && bulletCount_ <= 0) {
		isAvailable_ = false; // 弾がなくなったら使用不可
		return;
	}

	//リロード中かどうか
	if(isReloading_) {

		reloadTime_ -= TakeCFrameWork::GetDeltaTime();

		if( reloadTime_ <= 0.0f) {
			reloadTime_ = 0.0f; // リロード時間をリセット
			//リロード完了
			isReloading_ = false;
			bulletCount_ = magazineCount_; // 弾数をマガジン内の弾数にリセット
			remainingBulletCount_ -= magazineCount_; // 残弾数を更新
		}
		
	}

	//攻撃間隔の減少
	attackInterval_ -= TakeCFrameWork::GetDeltaTime();

	if (isBursting_) {
		burstInterval_ -= TakeCFrameWork::GetDeltaTime();
		if (burstInterval_ <= 0.0f && burstCount_ > 0) {
			// 弾発射
			if (ownerObject_->GetCharacterType() == CharacterType::PLAYER) {
				//プレイヤーの弾として発射
				bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,targetVel_, bulletSpeed_,damage_, CharacterType::PLAYER_BULLET);
			} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
				//エネミーの弾として発射
				bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,targetVel_, bulletSpeed_,damage_, CharacterType::ENEMY_BULLET);
			}

			bulletCount_--;
			if (bulletCount_ <= 0) {
				isReloading_ = true; // 弾がなくなったらリロード中にする
				reloadTime_ = maxReloadTime_; // リロード時間をリセット
			}

			burstCount_--;
			if (burstCount_ > 0) {
				burstInterval_ = kBurstAttackInterval; // 次の弾発射までの間隔を設定
			} else {
				isBursting_ = false; // 三連射終了
			}
		}
	}

	//親スケルトンのジョイントに追従させる
	if (parentSkeleton_ && !parentJointName_.empty()) {
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		object3d_->SetParent(*jointWorldMatrixOpt);
	}

	object3d_->Update();
}

//===================================================================================
//　ImGui更新処理
//===================================================================================
void Rifle::UpdateImGui() {

	ImGui::SeparatorText("Rifle");
	ImGui::Text("Charge Time: %.2f", chargeTime_);
	ImGui::Text("required Charge Time: %.2f", requiredChargeTime_);
	ImGui::Text("Is Charging: %s", isCharging_ ? "Yes" : "No");
	ImGui::SliderFloat("##Rifle::Bullet Speed", &bulletSpeed_, 100.0f, 1000.0f);
	ImGui::Separator();
	if(ImGui::TreeNode("Rifle Settings")) {
		ImGui::Text("Weapon Type: Rifle");
		ImGui::Text("Attack Power: %d", damage_);
		ImGui::Text("Attack Interval: %.2f", attackInterval_);
		ImGui::Text("Bullet Count: %d", bulletCount_);
		ImGui::Text("Max Bullet Count: %d", maxBulletCount_);
		ImGui::Text("Bullet Speed: %.2f", bulletSpeed_);
		ImGui::Text("Is Charge Attack: %s", canChargeAttack_ ? "Yes" : "No");
		ImGui::Text("Is Move Shootable: %s", canMoveShootable_ ? "Yes" : "No");
		ImGui::Text("Is Stop Shoot Only: %s", isStopShootOnly_ ? "Yes" : "No");
		ImGui::TreePop();
	}
}

//===================================================================================
//　描画処理
//===================================================================================
void Rifle::Draw() {

	object3d_->Draw();
}

//=====================================================================================
// 攻撃処理
//=====================================================================================
void Rifle::Attack() {

	if (isReloading_) {
		return; // リロード中は攻撃しない
	}

	//攻撃間隔が経過している場合
	if (attackInterval_ >= 0.0f) {
		return;
	}

	if (ownerObject_->GetCharacterType() == CharacterType::PLAYER){
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,targetVel_,bulletSpeed_,damage_, CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,targetVel_,bulletSpeed_,damage_, CharacterType::ENEMY_BULLET);
	} else {
		return; // キャラクタータイプが不明な場合は攻撃しない
	}

	bulletCount_--;
	if (bulletCount_ <= 0) {
		isReloading_ = true; // 弾がなくなったらリロード中にする
		reloadTime_ = maxReloadTime_; // リロード時間をリセット
	}
	//攻撃間隔のリセット
	attackInterval_ = kAttackInterval;
	//攻撃力の設定
}

//=====================================================================================
// チャージ攻撃処理
//=====================================================================================

void Rifle::Charge(float deltaTime) {
	// チャージ攻撃フラグが立っていない場合、チャージを開始
	if (!isCharging_) {
		isCharging_ = true;
		chargeTime_ = 0.0f;
	}
	chargeTime_ += deltaTime;

	// チャージ時間が最大に達した場合
	if (chargeTime_ >= requiredChargeTime_) { 

		// チャージ時間を最大に制限
		chargeTime_ = requiredChargeTime_; 
	}

	//TODO: チャージ中のエフェクトやアニメーションをここで処理する

}

//=====================================================================================
// チャージ攻撃実行
//=====================================================================================

void Rifle::ChargeAttack() {
	// チャージ攻撃が開始されていない場合は何もしない
	if(!isCharging_) return;
	isCharging_ = false;

	if (chargeTime_ >= requiredChargeTime_) {
		// 最大チャージ時間に達した場合の処理
		//停止撃ちで三連射
		isBursting_ = true;
		burstCount_ = kMaxBurstCount; // 3連射のカウントを初期化
		burstInterval_ = 0.0f; // 3連射の間隔を初期化

	} else {
		// チャージ時間が最大に達していない場合の処理
		// 通常の攻撃を1回行う
		Attack();
	}

	
	//チャージ時間のリセット
	chargeTime_ = 0.0f;
}

//=====================================================================================
// 所有者の設定
//=====================================================================================
void Rifle::SetOwnerObject(GameCharacter* owner) {
	ownerObject_ = owner;
}

//チャージ攻撃可能か
bool Rifle::IsChargeAttack() const {
	return canChargeAttack_;
}

//移動撃ち可能か
bool Rifle::IsMoveShootable() const {
	return canMoveShootable_;
}

//停止撃ち専用か
bool Rifle::IsStopShootOnly() const {
	return isStopShootOnly_;
}
