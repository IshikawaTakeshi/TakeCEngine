#include "VerticalMissileLauncher.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/MatrixMath.h"
#include "application/Weapon/Bullet/BulletManager.h"

void VerticalMissileLauncher::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) {
	bulletManager_ = bulletManager;

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	// ライフルの色を設定
	object3d_->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.5f, 0.5f, 0.0f, 1.0f });

	//武器の初期化
	weaponType_ = WeaponType::WEAPON_TYPE_RIFLE;
	damage_ = 400.0f;
	attackInterval_ = 0.0f;
	bulletSpeed_ = 500.0f; // 弾のスピードを設定

	maxBulletCount_ = 120;                   // 最大弾数
	remainingBulletCount_ = maxBulletCount_; // 残弾数を最大弾数に設定
	magazineCount_ = 3;                      // マガジン内の弾数
	bulletCount_ = magazineCount_;           // 初期弾数をマガジン内の弾数に設定

	maxReloadTime_ = 2.0f;

	canChargeAttack_ = false; // ライフルはチャージ攻撃不可
	canMoveShootable_ = true; // ライフルは移動撃ち可能
	isStopShootOnly_ = false; // ライフルは停止撃ち専用ではない
}

void VerticalMissileLauncher::Update() {

	if(remainingBulletCount_ <= 0 && bulletCount_ <= 0) {
		isAvailable_ = false; // 弾がなくなったら使用不可
		return;
	}

	//リロード中かどうか
	if(isReloading_ == true) {

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

	if (isBursting_) {
		burstInterval_ -= TakeCFrameWork::GetDeltaTime();
		if (burstInterval_ <= 0.0f && burstCount_ > 0) {
			// 弾発射
			if (ownerObject_->GetCharacterType() == CharacterType::PLAYER) {
				bulletManager_->ShootMissile(this, bulletSpeed_,damage_, CharacterType::PLAYER_MISSILE);
			} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
				bulletManager_->ShootMissile(this, bulletSpeed_,damage_, CharacterType::ENEMY_MISSILE);
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

void VerticalMissileLauncher::UpdateImGui() {

#ifdef _DEBUG
	ImGui::SeparatorText("VerticalMissile");
	ImGui::Text("Charge Time: %.2f", chargeTime_);
	ImGui::Text("Required Charge Time: %.2f", requiredChargeTime_);
	ImGui::Text("Is Charging: %s", isCharging_ ? "Yes" : "No");
	ImGui::SliderFloat("Bullet Speed", &bulletSpeed_, 100.0f, 1000.0f);
	Vector3 rotate = object3d_->GetTransform().rotate;
	ImGui::DragFloat3("Launcher::Rotate", &rotate.x, 0.01f);
	object3d_->SetRotate(rotate);
	ImGui::Separator();
	if (ImGui::TreeNode("VerticalMissile Settings")) {
		ImGui::Text("Weapon Type: VerticalMissile");
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
#endif // _DEBUG
}

void VerticalMissileLauncher::Draw() {

	// モデル描画
	object3d_->Draw();

}

void VerticalMissileLauncher::Attack() {

	if (isReloading_ == true) {
		return;
	}

	if (attackInterval_ <= 0.0f && bulletCount_ > 0) {
		// 弾発射
		//停止撃ちで三連射
		isBursting_ = true;
		burstCount_ = kMaxBurstCount; // 3連射のカウントを初期化
		burstInterval_ = 0.0f; // 3連射の間隔を初期化

		//攻撃間隔のリセット
		attackInterval_ = maxReloadTime_;
	}
}

void VerticalMissileLauncher::Charge(float deltaTime) {
	deltaTime;
}

void VerticalMissileLauncher::ChargeAttack() {}

void VerticalMissileLauncher::SetOwnerObject(GameCharacter* owner) {
	ownerObject_ = owner;
}

bool VerticalMissileLauncher::IsChargeAttack() const {
	// バーティカルミサイルはチャージ攻撃不可
	return canChargeAttack_;
}

bool VerticalMissileLauncher::IsMoveShootable() const {
	//移動撃ち可能
	return canMoveShootable_;
}

bool VerticalMissileLauncher::IsStopShootOnly() const {
	//停止撃ち専用ではない
	return isStopShootOnly_;
}
