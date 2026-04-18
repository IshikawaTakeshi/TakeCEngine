#include "ShotGun.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/Vector3Math.h"
#include "engine/Math/MathEnv.h"
#include "application/Weapon/Bullet/BulletManager.h"

//====================================================================
// 初期化
//====================================================================
void ShotGun::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {

	shotSE_ = AudioManager::GetInstance().LoadSound("SE/ShotGun.mp3");
	seVolume_ = 0.2f;

	bulletManager_ = bulletManager;

	//武器の初期化
	weaponData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<WeaponData>("ShotGun.json");

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, weaponData_.modelFilePath);

	// ライフルの色を設定
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.5f, 0.5f, 0.0f, 1.0f });
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetEnvCoefficient(0.8f);

	shotGunInfo_ = std::get<ShotGunInfo>(weaponData_.actionData);
	weaponState_.attackInterval = weaponData_.config.attackInterval;
	weaponState_.bulletCount = weaponData_.config.maxMagazineCount;
	weaponState_.remainingBulletCount = weaponData_.config.maxBulletCount; // 残弾数を最大弾数に設定
}

void ShotGun::Update() {

	if (weaponState_.remainingBulletCount <= 0 && weaponState_.bulletCount <= 0) {
		weaponState_.isAvailable = false; // 弾がなくなったら使用不可
		return;
	}

	//リロード中かどうか
	if (weaponState_.isReloading) {

		weaponState_.reloadTime -= TakeCFrameWork::GetDeltaTime();

		if (weaponState_.reloadTime <= 0.0f) {
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
		//Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, ownerObject_->GetObject3d()->GetWorldMatrix());
		if (jointWorldMatrixOpt.has_value()) {
			object3d_->SetParent(jointWorldMatrixOpt.value());
		} else {
			assert(false && "Failed to get joint world matrix for ShotGun.");
		}
	}

	object3d_->Update();
}

void ShotGun::UpdateImGui() {

	ImGui::SeparatorText("ShotGun Status");
	ImGui::SliderFloat("##ShotGun::Bullet Speed", &weaponData_.config.bulletSpeed, 100.0f, 1000.0f);
	ImGui::Separator();
	weaponData_.config.EditConfigImGui(weaponData_.weaponName);
	shotGunInfo_.EditConfigImGui();

	if (ImGui::Button("Save ShotGun Data")) {
		weaponData_.actionData = shotGunInfo_;
		TakeCFrameWork::GetJsonLoader()->SaveJsonData("ShotGun.json", weaponData_);
	}
}

void ShotGun::Draw() {
	if (weaponState_.remainingBulletCount <= 0 && weaponState_.bulletCount <= 0) {
		return; // 弾がなくなったら描画しない
	}
	object3d_->Draw();
}

void ShotGun::Attack() {
	if (weaponState_.isReloading) {
		return; // リロード中は攻撃しない
	}

	//攻撃間隔が経過している場合
	if (weaponState_.attackInterval >= 0.0f) {
		return;
	}

	Vector3 muzzlePos = GetCenterPosition(); // or joint-based muzzle
	Vector3 baseDir = (GetTargetPos() - muzzlePos).Normalize(); // 基本の射撃方向
	CharacterType ownerType = ownerObject_->GetCharacterType();
	//弾の発射
	for (size_t i = 0; i < shotGunInfo_.pelletCount; ++i) {
		float yawOffset   = GetRandomFloat(-shotGunInfo_.spreadDeg, shotGunInfo_.spreadDeg);
		float pitchOffset = GetRandomFloat(-shotGunInfo_.spreadDeg, shotGunInfo_.spreadDeg);
		
		// ランダムな方向ベクトルを計算
		Vector3 dir = Vector3Math::ApplyYawPitch(baseDir, yawOffset, pitchOffset).Normalize();

		// Bullet 生成
		bulletManager_->ShootBullet(
			object3d_->GetCenterPosition(),
			dir,
			weaponData_.config.bulletSpeed,
			weaponData_.config.power,
			static_cast<CharacterType>(static_cast<int>(ownerType) + 1));
	}

	weaponState_.bulletCount--;
	if (weaponState_.bulletCount <= 0) {
		weaponState_.isReloading = true; // 弾がなくなったらリロード中にする
		weaponState_.reloadTime = weaponData_.config.maxReloadTime; // リロード時間をリセット
	}
	AudioManager::GetInstance().SoundPlayWave(shotSE_, seVolume_);
	//攻撃間隔のリセット
	weaponState_.attackInterval = weaponData_.config.attackInterval;
}