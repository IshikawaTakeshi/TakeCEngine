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
	weaponData_.weaponName = "Rifle";
	weaponData_.weaponType = WeaponType::WEAPON_TYPE_RIFLE;
	weaponData_.modelFilePath = filePath;
	weaponData_.config.power = 90.0f;
	weaponData_.config.kAttackInterval = 0.3f; // 攻撃間隔定数を設定
	weaponData_.config.bulletSpeed = 550.0f; // 弾のスピードを設定
	weaponData_.config.maxMagazineCount = 30; // マガジン内の弾数を設定
	weaponData_.config.maxBulletCount = 600;
	weaponData_.config.effectiveRange = 700.0f; // 有効射程距離を設定
	weaponState_.attackInterval = rifleInfo_.burstShotInfo.kInterval; // 連射の間隔を攻撃間隔に設定
	weaponState_.bulletCount = weaponData_.config.maxMagazineCount;
	weaponState_.remainingBulletCount = weaponData_.config.maxBulletCount; // 残弾数を最大弾数に設定

	weaponData_.config.maxReloadTime = 3.0f; // 最大リロード時間を設定

	weaponData_.config.canChargeAttack = false; // ライフルはチャージ攻撃可能
	weaponData_.config.canMoveShootable = true; // ライフルは移動撃ち可能
	weaponData_.config.isStopShootOnly = false; // ライフルは停止撃ち専用ではない
}

//===================================================================================
//　更新処理
//===================================================================================
void Rifle::Update() {

	if(weaponState_.remainingBulletCount <= 0 && weaponState_.bulletCount <= 0) {
		weaponState_.isAvailable = false; // 弾がなくなったら使用不可
		return;
	}

	//リロード中かどうか
	if(weaponState_.isReloading) {

		weaponState_.reloadTime -= TakeCFrameWork::GetDeltaTime();

		if( weaponState_.reloadTime <= 0.0f) {
			weaponState_.reloadTime = 0.0f; // リロード時間をリセット
			//リロード完了
			weaponState_.isReloading = false;
			weaponState_.bulletCount = weaponData_.config.maxMagazineCount; // 弾数をマガジン内の弾数にリセット
			weaponState_.remainingBulletCount -= weaponData_.config.maxMagazineCount; // 残弾数を更新
		}
		
	}

	//攻撃間隔の減少
	weaponState_.attackInterval -= TakeCFrameWork::GetDeltaTime();

	if (burstShotState_.isActive) {
		burstShotState_.intervalTimer -= TakeCFrameWork::GetDeltaTime();
		if (burstShotState_.intervalTimer <= 0.0f && burstShotState_.count > 0) {
			// 弾発射
			if (ownerObject_->GetCharacterType() == CharacterType::PLAYER) {
				//プレイヤーの弾として発射
				bulletManager_->ShootBullet(
					object3d_->GetCenterPosition(),
					targetPos_,targetVel_,
					weaponData_.config.bulletSpeed,
					weaponData_.config.power,
					CharacterType::PLAYER_BULLET);

			} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
				//エネミーの弾として発射
				bulletManager_->ShootBullet(
					object3d_->GetCenterPosition(),
					targetPos_,targetVel_,
					weaponData_.config.bulletSpeed,
					weaponData_.config.power,
					CharacterType::ENEMY_BULLET);
			}

			//弾数の減少処理
			weaponState_.bulletCount--;
			if (weaponState_.bulletCount <= 0) {
				weaponState_.isReloading = true; // 弾がなくなったらリロード中にする
				weaponState_.reloadTime = weaponData_.config.maxReloadTime; // リロード時間をリセット
			}

			burstShotState_.count--;
			if (burstShotState_.count > 0) {
				burstShotState_.intervalTimer = rifleInfo_.burstShotInfo.kInterval; // 次の弾発射までの間隔を設定
			} else {
				burstShotState_.isActive = false; // 三連射終了
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
	ImGui::Text("Charge Time: %.2f", weaponState_.chargeTime);
	ImGui::Text("required Charge Time: %.2f", weaponData_.config.requiredChargeTime);
	ImGui::Text("Is Charging: %s", weaponState_.isCharging ? "Yes" : "No");
	ImGui::SliderFloat("##Rifle::Bullet Speed", &weaponData_.config.bulletSpeed, 100.0f, 1000.0f);
	ImGui::Separator();
	rifleInfo_.burstShotInfo.EditConfigImGui();
	weaponData_.config.EditConfigImGui();

	if(ImGui::Button("Save Rifle Config")) {
		// 設定をJSONに保存
		weaponData_.actionData = rifleInfo_;
		TakeCFrameWork::GetJsonLoader()->SaveJsonData("Rifle.json", weaponData_);

	}
}

//===================================================================================
//　描画処理
//===================================================================================
void Rifle::Draw() {

	if(weaponState_.remainingBulletCount <= 0 && weaponState_.bulletCount <= 0) {
		return; // 弾がなくなったら描画しない
	}
	object3d_->Draw();
}

//=====================================================================================
// 攻撃処理
//=====================================================================================
void Rifle::Attack() {

	if (weaponState_.isReloading) {
		return; // リロード中は攻撃しない
	}

	//攻撃間隔が経過している場合
	if (weaponState_.attackInterval >= 0.0f) {
		return;
	}

	//弾の発射
	if (ownerObject_->GetCharacterType() == CharacterType::PLAYER){
		bulletManager_->ShootBullet(
			object3d_->GetCenterPosition(),
			targetPos_,
			targetVel_,
			weaponData_.config.bulletSpeed,
			weaponData_.config.power,
			CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(
			object3d_->GetCenterPosition(),
			targetPos_,targetVel_,
			weaponData_.config.bulletSpeed,
			weaponData_.config.power,
			CharacterType::ENEMY_BULLET);
	} else {
		return; // キャラクタータイプが不明な場合は攻撃しない
	}

	weaponState_.bulletCount--;
	if (weaponState_.bulletCount <= 0) {
		weaponState_.isReloading = true; // 弾がなくなったらリロード中にする
		weaponState_.reloadTime = weaponData_.config.maxReloadTime; // リロード時間をリセット
	}
	//攻撃間隔のリセット
	weaponState_.attackInterval = weaponData_.config.kAttackInterval;
	//攻撃力の設定
}

//=====================================================================================
// チャージ攻撃処理
//=====================================================================================

void Rifle::Charge(float deltaTime) {
	// チャージ攻撃フラグが立っていない場合、チャージを開始
	if (weaponState_.isCharging == false) {
		weaponState_.isCharging = true;
		weaponState_.chargeTime = 0.0f;
	}
	weaponState_.chargeTime += deltaTime;

	// チャージ時間が最大に達した場合
	if (weaponState_.chargeTime >= weaponData_.config.requiredChargeTime) { 

		// チャージ時間を最大に制限
		weaponState_.chargeTime = weaponData_.config.requiredChargeTime; 
	}

	//TODO: チャージ中のエフェクトやアニメーションをここで処理する

}

//=====================================================================================
// チャージ攻撃実行
//=====================================================================================

void Rifle::ChargeAttack() {
	// チャージ攻撃が開始されていない場合は何もしない
	if (weaponState_.isCharging == false) {
		return;
	}
	weaponState_.isCharging = false;

	if (weaponState_.chargeTime >= weaponData_.config.requiredChargeTime) {
		// 最大チャージ時間に達した場合の処理
		//停止撃ちで三連射
		burstShotState_.isActive = true;
		burstShotState_.count = rifleInfo_.burstShotInfo.kMaxBurstCount; // 3連射のカウントを初期化
		burstShotState_.intervalTimer = 0.0f; // 3連射の間隔を初期化

	} else {
		// チャージ時間が最大に達していない場合の処理
		// 通常の攻撃を1回行う
		Attack();
	}

	
	//チャージ時間のリセット
	weaponState_.chargeTime = 0.0f;
}

//=====================================================================================
// 所有者の設定
//=====================================================================================
void Rifle::SetOwnerObject(GameCharacter* owner) {
	ownerObject_ = owner;
}

//チャージ攻撃可能か
bool Rifle::IsChargeAttack() const {
	return weaponData_.config.canChargeAttack;
}

//移動撃ち可能か
bool Rifle::IsMoveShootable() const {
	return weaponData_.config.canMoveShootable;
}

//停止撃ち専用か
bool Rifle::IsStopShootOnly() const {
	return weaponData_.config.isStopShootOnly;
}