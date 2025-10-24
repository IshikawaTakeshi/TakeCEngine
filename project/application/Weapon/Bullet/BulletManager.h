#pragma once
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/Bullet/VerticalMissile.h"
#include "Weapon/Bullet/BulletPool.h"
#include "Weapon/Bullet/MissilePool.h"
#include "3d/Object3dCommon.h"
#include <string>
#include <memory>
#include <vector>

//============================================================================
// BulletManager class
//============================================================================
class BulletManager {
public:

	BulletManager() = default;
	~BulletManager() = default;

	//=====================================================================
	// functions
	//=====================================================================

	//初期化
	void Initialize(Object3dCommon* object3dCommon,size_t size);
	//開放処理
	void Finalize();
	//更新
	void Update();
	//ImGuiの更新
	void Draw();
	//コライダー描画
	void DrawCollider();

	//弾の発射処理
	void ShootBullet(const Vector3& weaponPos, const Vector3& targetPos, const float& speed,float damage, CharacterType type);
	//ミサイルの発射処理
	void ShootMissile(BaseWeapon* ownerWeapon, const float& speed,float damage, CharacterType type);

	//----- getter ---------------------------
	
	//全ての弾を取得
	std::vector<Bullet*> GetAllBullets();
	//全てのミサイルを取得
	std::vector<VerticalMissile*> GetAllMissiles();

private:

	Object3dCommon* object3dCommon_ = nullptr; // 3Dオブジェクト共通クラス
	std::string bulletFilePath_;               // 弾のファイルパス
	std::string missileFilePath_;              // ミサイルのファイルパス
	std::unique_ptr<BulletPool> bulletPool_;   // 弾プール
	std::unique_ptr<MissilePool> missilePool_; // ミサイルプール
};