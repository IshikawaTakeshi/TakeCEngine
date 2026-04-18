#include "Bazooka.h"
#include "base/TakeCFrameWork.h"
#include "base/ImGuiManager.h"
#include "math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

//=============================================================================
// 初期化処理
//=============================================================================
void Bazooka::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager) {

	shotSE_ = AudioManager::GetInstance().LoadSound("SE/ShotBazooka.mp3");
	seVolume_ = 0.2f;

	//弾薬マネージャの設定
	bulletManager_ = bulletManager;
	//武器の初期化
	weaponData_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<WeaponData>("Bazooka.json");
	//3Dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, weaponData_.modelFilePath);


	// ライフルの色を設定
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.5f, 0.5f, 0.0f, 1.0f });
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetEnvCoefficient(0.8f);

	weaponState_.attackInterval = weaponData_.config.attackInterval; // 攻撃間隔を設定
	weaponState_.bulletCount = weaponData_.config.maxMagazineCount;           // 初期弾数を設定
	weaponState_.remainingBulletCount = weaponData_.config.maxBulletCount; // 残弾数を最大弾数に設定

	muzzleFlashEmitter_ = std::make_unique<ParticleEmitter>();
	muzzleFlashEmitter_->Initialize("RifleMuzzleFlashEmitter","RifleMuzzleFlash2.json");
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
			weaponState_.bulletCount = weaponData_.config.maxMagazineCount; // 弾数をマガジン内の弾数にリセット
			weaponState_.remainingBulletCount -= weaponData_.config.maxMagazineCount; // 残弾数を減らす
		}

	}

	//攻撃間隔の減少
	weaponState_.attackInterval -= TakeCFrameWork::GetDeltaTime();

	//親スケルトンのジョイントに追従させる
	if (parentSkeleton_ && !parentJointName_.empty()) {
		const Matrix4x4& characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		object3d_->SetParent(jointWorldMatrixOpt.value());
	}

	//3Dオブジェクトの更新
	object3d_->Update();

	muzzleFlashEmitter_->SetTranslate(object3d_->GetWorldPosition());
	muzzleFlashEmitter_->Update();
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void Bazooka::UpdateImGui() {
	ImGui::SeparatorText("Bazooka Settings");
	weaponData_.config.EditConfigImGui(weaponData_.weaponName);

	if(ImGui::Button("Save Bazooka Config")) {
		// 設定をJSONに保存

		TakeCFrameWork::GetJsonLoader()->SaveJsonData("Bazooka.json", weaponData_);
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void Bazooka::Draw() {
	if(weaponState_.isAvailable == false) {
		return; // 使用不可なら描画しない
	}
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
		bulletManager_->ShootBazookaBullet(
			object3d_->GetCenterPosition(),
			targetPos_,targetVelocity_,
			weaponData_.config.bulletSpeed,
			weaponData_.config.power,
			CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBazookaBullet(
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

	muzzleFlashEmitter_->Emit();
	AudioManager::GetInstance().SoundPlayWave(shotSE_, seVolume_);
	//攻撃間隔のリセット
	weaponState_.attackInterval = weaponData_.config.attackInterval;
}