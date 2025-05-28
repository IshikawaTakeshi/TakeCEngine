#include "Rifle.h"
#include "TakeCFrameWork.h"

//最大弾数の設定


Rifle::~Rifle() {}

void Rifle::Initialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& filePath) {

	bulletManager_ = bulletManager;

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//武器の初期化
	weaponType_ = static_cast<int32_t>(WeaponType::WEAPON_TYPE_RIFLE);
	attackPower_ = 10;
	attackInterval_ = kAttackInterval;
	bulletCount_ = 30;
	maxBulletCount_ = 30;

}

void Rifle::Update() {

	//攻撃間隔の減少
	attackInterval_ -= TakeCFrameWork::GetDeltaTime();


	object3d_->Update();
}

void Rifle::Draw() {

	object3d_->Draw();
}

void Rifle::Attack() {

	//攻撃間隔が経過している場合
	if (attackInterval_ >= 0.0f) {
		return;
	}

	if (ownerObject_->GetCharacterType() == CharacterType::PLAYER){
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_, CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_, CharacterType::ENEMY_BULLET);
	} else {
		return; // キャラクタータイプが不明な場合は攻撃しない
	}

	//弾数の減少
	bulletCount_--;
	if (bulletCount_ <= 0) {
		bulletCount_ = maxBulletCount_;
	}
	//攻撃間隔のリセット
	attackInterval_ = kAttackInterval;
	//攻撃力の設定
}

void Rifle::SetOwnerObject(GameCharacter* owner) {
	BaseWeapon::SetOwnerObject(owner);
	object3d_->SetParent(owner->GetObject3d());
}
