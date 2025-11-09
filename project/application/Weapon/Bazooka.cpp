#include "Bazooka.h"
#include "base/TakeCFrameWork.h"
#include "base/ImGuiManager.h"
#include "math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

//=============================================================================
// 初期化処理
//=============================================================================
void Bazooka::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) {

	//弾薬マネージャの設定
	bulletManager_ = bulletManager;
	//3Dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	// ライフルの色を設定
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.5f, 0.5f, 0.0f, 1.0f });
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetEnvCoefficient(0.8f);

	//武器の初期化
	weaponType_ = WeaponType::WEAPON_TYPE_BAZOOKA;
	weaponData_.power = 1500.0f;                 // 攻撃力を設定
	weaponData_.kAttackInterval = 2.0f;         // 攻撃間隔定数を設定
	weaponState_.attackInterval = weaponData_.kAttackInterval; // 攻撃間隔を設定
	weaponData_.bulletSpeed = 500.0f;             // 弾のスピードを設定
	weaponData_.effectiveRange = 1000.0f;        // 有効射程距離を設定

	weaponData_.maxMagazineCount = 5;                      // マガジン内の弾数を設定
	weaponState_.bulletCount = weaponData_.maxMagazineCount;           // 初期弾数を設定
	weaponData_.maxBulletCount = 50;                    // 最大弾数を設定
	weaponState_.remainingBulletCount = weaponData_.maxBulletCount; // 残弾数を最大弾数に設定

	weaponData_.maxReloadTime = 4.0f;

	weaponData_.canChargeAttack = false;  // バズーカはチャージ攻撃不可
	weaponData_.canMoveShootable = false; // バズーカは移動撃ち不可
	weaponData_.isStopShootOnly = true;  // バズーカは停止撃ち専用
}

//=============================================================================
// 更新処理
//=============================================================================
void Bazooka::Update() {

	if(weaponState_.remainingBulletCount <= 0 && weaponState_.bulletCount <= 0) {
		weaponState_.isAvailable = false; // 弾がなくなったら使用不可
		return;
	}


	//リロード中かどうか
	if(weaponState_.isReloading == true) {

		weaponState_.reloadTime -= TakeCFrameWork::GetDeltaTime();

		if( weaponState_.reloadTime <= 0.0f) {
			weaponState_.reloadTime = 0.0f; // リロード時間をリセット
			//リロード完了
			weaponState_.isReloading = false;
			weaponState_.bulletCount = weaponData_.maxMagazineCount; // 弾数をマガジン内の弾数にリセット
			weaponState_.remainingBulletCount -= weaponData_.maxMagazineCount; // 残弾数を減らす
		}

	}

	//攻撃間隔の減少
	weaponState_.attackInterval -= TakeCFrameWork::GetDeltaTime();

	//親スケルトンのジョイントに追従させる
	if (parentSkeleton_ && !parentJointName_.empty()) {
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		object3d_->SetParent(*jointWorldMatrixOpt);
	}

	//3Dオブジェクトの更新
	object3d_->Update();
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void Bazooka::UpdateImGui() {
	ImGui::SeparatorText("Bazooka Settings");
	weaponData_.EditConfigImGui();

	if(ImGui::Button("Save Bazooka Config")) {
		// 設定をJSONに保存
		TakeCFrameWork::GetJsonLoader()->SaveJsonData("BazookaConfig.json", weaponData_);
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void Bazooka::Draw() {

	object3d_->Draw();
}

//=============================================================================
// 攻撃処理
//=============================================================================
void Bazooka::Attack() {

	if (weaponState_.isReloading == true) {
		return;
	}

	//攻撃間隔が経過している場合
	if (weaponState_.attackInterval >= 0.0f) {
		return;
	}

	//弾の発射
	if (ownerObject_->GetCharacterType() == CharacterType::PLAYER){
		bulletManager_->ShootBullet(
			object3d_->GetCenterPosition(),
			targetPos_,targetVel_,
			weaponData_.bulletSpeed,
			weaponData_.power,
			CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(
			object3d_->GetCenterPosition(),
			targetPos_,targetVel_,
			weaponData_.bulletSpeed,
			weaponData_.power,
			CharacterType::ENEMY_BULLET);
	} else {
		return; // キャラクタータイプが不明な場合は攻撃しない
	}

	weaponState_.bulletCount--;
	if (weaponState_.bulletCount <= 0) {
		weaponState_.isReloading = true; // 弾がなくなったらリロード中にする
		weaponState_.reloadTime = weaponData_.maxReloadTime; // リロード時間をリセット
	}
	//攻撃間隔のリセット
	weaponState_.attackInterval = weaponData_.kAttackInterval;
}

//=============================================================================
// 所有者の設定
//=============================================================================
void Bazooka::SetOwnerObject(GameCharacter* owner) {

	ownerObject_ = owner;
}

// チャージ攻撃可能か
bool Bazooka::IsChargeAttack() const {
	// バズーカはチャージ攻撃不可
	return weaponData_.canChargeAttack;
}

// 移動撃ち可能か
bool Bazooka::IsMoveShootable() const {
	// バズーカは移動撃ち不可
	return weaponData_.canMoveShootable;
}

// 停止撃ち専用か
bool Bazooka::IsStopShootOnly() const {
	// バズーカは停止撃ち専用
	return weaponData_.isStopShootOnly;
}