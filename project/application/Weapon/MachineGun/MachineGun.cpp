#include "MachineGun.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

//===================================================================================
//　初期化処理
//===================================================================================
void MachineGun::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {

	bulletManager_ = bulletManager;

	//武器の初期化
	weaponData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<WeaponData>("MachineGun.json");
	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, weaponData_.modelFilePath);

	weaponState_.attackInterval = weaponData_.config.attackInterval; // 攻撃間隔を設定
	weaponState_.bulletCount = weaponData_.config.maxMagazineCount;
	weaponState_.remainingBulletCount = weaponData_.config.maxBulletCount; // 残弾数を最大弾数に設定
}

//===================================================================================
//　更新処理
//===================================================================================
void MachineGun::Update() {
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

	//親スケルトンのジョイントに追従させる
	if (parentSkeleton_ && !parentJointName_.empty()) {
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, ownerObject_->GetObject3d()->GetWorldMatrix());
		if (jointWorldMatrixOpt.has_value()) {
			object3d_->SetParent(jointWorldMatrixOpt.value());
		} else {
			assert(false && "Failed to get joint world matrix for MachineGun.");
		}
	}

	object3d_->Update();
}

//===================================================================================
//　ImGui更新処理
//===================================================================================
void MachineGun::UpdateImGui() {

	ImGui::SeparatorText("MachineGun");
	weaponData_.config.EditConfigImGui(weaponData_.weaponName);
	if(ImGui::Button("Save MachineGun Config")) {
		// 設定をJSONに保存
		TakeCFrameWork::GetJsonLoader()->SaveJsonData("MachineGun.json", weaponData_);
	}
}

//===================================================================================
//　描画処理
//===================================================================================
void MachineGun::Draw() {
	if(weaponState_.remainingBulletCount <= 0 && weaponState_.bulletCount <= 0) {
		return; // 弾がなくなったら描画しない
	}
	object3d_->Draw();
}

//===================================================================================
//　攻撃処理
//===================================================================================
void MachineGun::Attack() {
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
			targetVelocity_,
			weaponData_.config.bulletSpeed,
			weaponData_.config.power,
			CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(
			object3d_->GetCenterPosition(),
			targetPos_,targetVelocity_,
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
	weaponState_.attackInterval = weaponData_.config.attackInterval;
}