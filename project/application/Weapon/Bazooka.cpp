#include "Bazooka.h"
#include "base/TakeCFrameWork.h"
#include "base/ImGuiManager.h"
#include "math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

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
	damage_ = 1500.0f;                 // 攻撃力を設定
	attackInterval_ = kAttackInterval; // 攻撃間隔を設定
	bulletSpeed_ = 500.0f;             // 弾のスピードを設定
	effectiveRange_ = 800.0f;        // 有効射程距離を設定

	magazineCount_ = 5;                      // マガジン内の弾数を設定
	bulletCount_ = magazineCount_;           // 初期弾数を設定
	maxBulletCount_ = 50;                    // 最大弾数を設定
	remainingBulletCount_ = maxBulletCount_; // 残弾数を最大弾数に設定

	maxReloadTime_ = 4.0f;

	canChargeAttack_ = false;  // バズーカはチャージ攻撃不可
	canMoveShootable_ = false; // バズーカは移動撃ち不可
	isStopShootOnly_ = true;  // バズーカは停止撃ち専用
}

void Bazooka::Update() {

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
			remainingBulletCount_ -= magazineCount_; // 残弾数を減らす
		}

	}

	//攻撃間隔の減少
	attackInterval_ -= TakeCFrameWork::GetDeltaTime();

	//親スケルトンのジョイントに追従させる
	if (parentSkeleton_ && !parentJointName_.empty()) {
		Matrix4x4 characterWorldMatrix = ownerObject_->GetObject3d()->GetWorldMatrix();
		auto jointWorldMatrixOpt = parentSkeleton_->GetJointWorldMatrix(parentJointName_, characterWorldMatrix);
		object3d_->SetParent(*jointWorldMatrixOpt);
	}

	//3Dオブジェクトの更新
	object3d_->Update();
}

void Bazooka::UpdateImGui() {
	ImGui::SeparatorText("Bazooka Settings");
	ImGui::Text("Weapon Type: Bazooka");
	ImGui::Text("Transform: (%.2f, %.2f, %.2f)", 
		object3d_->GetTransform().translate.x, 
		object3d_->GetTransform().translate.y, 
		object3d_->GetTransform().translate.z);
	
	ImGui::Text("Attack Power: %d", damage_);
	ImGui::Text("Attack Interval: %.2f", attackInterval_);
	ImGui::Text("Bullet Count: %d", bulletCount_);
	ImGui::Text("Max Bullet Count: %d", maxBulletCount_);
	ImGui::Text("Bullet Speed: %.2f", bulletSpeed_);
}

void Bazooka::Draw() {

	object3d_->Draw();
}

void Bazooka::Attack() {

	if (isReloading_ == true) {
		return;
	}

	//攻撃間隔が経過している場合
	if (attackInterval_ >= 0.0f) {
		return;
	}

	if (ownerObject_->GetCharacterType() == CharacterType::PLAYER){
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,bulletSpeed_,damage_, CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,bulletSpeed_,damage_, CharacterType::ENEMY_BULLET);
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
}

void Bazooka::SetOwnerObject(GameCharacter* owner) {

	ownerObject_ = owner;
}

bool Bazooka::IsChargeAttack() const {
	// バズーカはチャージ攻撃不可
	return canChargeAttack_;
}

bool Bazooka::IsMoveShootable() const {
	// バズーカは移動撃ち不可
	return canMoveShootable_;
}

bool Bazooka::IsStopShootOnly() const {
	// バズーカは停止撃ち専用
	return isStopShootOnly_;
}
