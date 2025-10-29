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

	//=====================================================================
	// functions
	//=====================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BulletManager() = default;
	~BulletManager() = default;

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

	/// <summary>
	/// 弾の発射処理
	/// </summary>
	/// <param name="weaponPos"> 武器の位置</param>
	/// <param name="targetPos"> ターゲットの位置</param>
	/// <param name="speed">     弾の速度</param>
	/// <param name="damage">    ダメージ量</param>
	/// <param name="type">      キャラクタータイプ</param>
	void ShootBullet(const Vector3& weaponPos, const Vector3& targetPos,const Vector3& targetVel, const float& speed,float damage, CharacterType type);
	
	/// <summary>
	/// ミサイルの発射処理
	/// </summary>
	/// <param name="ownerWeapon"> 所有武器</param>
	/// <param name="speed">       ミサイルの速度</param>
	/// <param name="damage">      ダメージ量</param>
	/// <param name="type">        キャラクタータイプ</param>
	void ShootMissile(BaseWeapon* ownerWeapon, float speed,float homingRate,float damage, CharacterType type);

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