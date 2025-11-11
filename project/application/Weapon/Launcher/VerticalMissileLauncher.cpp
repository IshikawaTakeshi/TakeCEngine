#include "VerticalMissileLauncher.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

//================================================================================
//　初期化
//================================================================================
void VerticalMissileLauncher::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) {
	bulletManager_ = bulletManager;

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	// ライフルの色を設定
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.5f, 0.5f, 0.0f, 1.0f });

	//武器の初期化
	weaponData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<WeaponData>("VMLauncher.json");
	vmLauncherInfo_ = std::get<VerticalMissileLauncherInfo>(weaponData_.actionData);
	weaponState_.bulletCount = weaponData_.config.maxMagazineCount;           // 初期弾数をマガジン内の弾数に設定
	weaponState_.attackInterval = 0.0f;
	weaponState_.remainingBulletCount = weaponData_.config.maxBulletCount; // 残弾数を最大弾数に設定
}

//================================================================================
//　更新
//================================================================================
void VerticalMissileLauncher::Update() {

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
			weaponState_.bulletCount = weaponData_.config.maxMagazineCount; // 弾数をマガジン内の弾数にリセット
			weaponState_.remainingBulletCount -= weaponData_.config.maxMagazineCount; // 残弾数を減らす
		}
	}
	//攻撃間隔の減少
	weaponState_.attackInterval -= TakeCFrameWork::GetDeltaTime();

	if (burstShotState_.isActive) {
		burstShotState_.intervalTimer -= TakeCFrameWork::GetDeltaTime();
		// 三連射の間隔タイマーが0以下で、まだ弾を撃つ回数が残っている場合
		if (burstShotState_.intervalTimer <= 0.0f && burstShotState_.count > 0) {
			// 弾発射
			if (ownerObject_->GetCharacterType() == CharacterType::PLAYER) {
				bulletManager_->ShootMissile(
					this,
					weaponData_.config.bulletSpeed,
					vmLauncherInfo_.homingRate,
					weaponData_.config.power,
					CharacterType::PLAYER_MISSILE);

			} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
				bulletManager_->ShootMissile(
					this,
					weaponData_.config.bulletSpeed,
					vmLauncherInfo_.homingRate,
					weaponData_.config.power,
					CharacterType::ENEMY_MISSILE);
			}
			weaponState_.bulletCount--;
			if (weaponState_.bulletCount <= 0) {
				weaponState_.isReloading = true; // 弾がなくなったらリロード中にする
				weaponState_.reloadTime = weaponData_.config.maxReloadTime; // リロード時間をリセット
			}

			burstShotState_.count--;
			if (burstShotState_.count > 0) {
				burstShotState_.intervalTimer = vmLauncherInfo_.burstShotInfo.kInterval; // 次の弾発射までの間隔を設定
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
//　ImGuiの更新
//===================================================================================
void VerticalMissileLauncher::UpdateImGui() {

	ImGui::SeparatorText("VerticalMissile");

	//WeaponDataのImGui表示
	weaponData_.config.EditConfigImGui();

	Vector3 rotate = object3d_->GetTransform().rotate;
	ImGui::DragFloat3("Launcher::Rotate", &rotate.x, 0.01f);
	object3d_->SetRotate(rotate);
	ImGui::SeparatorText("VerticalMissile Settings");

	//連射情報のImGui表示
	vmLauncherInfo_.burstShotInfo.EditConfigImGui();
	if (ImGui::Button("Save VerticalMissileLauncher config")) {
		// 設定をJSONに保存
		weaponData_.actionData = vmLauncherInfo_;
		TakeCFrameWork::GetJsonLoader()->SaveJsonData("VMLauncher.json", weaponData_);
	}
}

//================================================================================
//　描画処理
//================================================================================
void VerticalMissileLauncher::Draw() {

	// モデル描画
	object3d_->Draw();

}

//================================================================================
//　攻撃処理
//================================================================================
void VerticalMissileLauncher::Attack() {

	if (weaponState_.isReloading == true) {
		return;
	}

	if (weaponState_.attackInterval <= 0.0f && weaponState_.bulletCount > 0) {
		// 弾発射
		//停止撃ちで三連射
		burstShotState_.isActive = true;
		burstShotState_.count = vmLauncherInfo_.burstShotInfo.kMaxBurstCount; // 3連射のカウントを初期化
		burstShotState_.intervalTimer = 0.0f; // 3連射の間隔を初期化

		//攻撃間隔のリセット
		weaponState_.attackInterval = weaponData_.config.maxReloadTime;
	}
}


//================================================================================
//　チャージ処理
//================================================================================
void VerticalMissileLauncher::Charge(float deltaTime) {
	// チャージ処理なし
	deltaTime;
}

//================================================================================
//　チャージ攻撃処理
//================================================================================
void VerticalMissileLauncher::ChargeAttack() {
	// チャージ攻撃処理なし
}


//================================================================================
//　所有者の設定
//================================================================================
void VerticalMissileLauncher::SetOwnerObject(GameCharacter* owner) {
	ownerObject_ = owner;
}

//チャージ攻撃可能か
bool VerticalMissileLauncher::IsChargeAttack() const {
	// バーティカルミサイルはチャージ攻撃不可
	return weaponData_.config.canChargeAttack;
}

//移動撃ち可能か
bool VerticalMissileLauncher::IsMoveShootable() const {
	//移動撃ち可能
	return weaponData_.config.canMoveShootable;
}

//停止撃ち専用か
bool VerticalMissileLauncher::IsStopShootOnly() const {
	//停止撃ち専用ではない
	return weaponData_.config.isStopShootOnly;
}