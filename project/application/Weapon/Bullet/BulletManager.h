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
	// 描画
	void Draw();

	//コライダー描画
	void DrawCollider();

	/// <summary>
	/// 弾の発射処理
	/// </summary>
	/// <param name="weaponPos"> 武器の位置</param>
	/// <param name="targetPos"> ターゲットの位置</param>
	/// <param name="speed">     弾の速度</param>
	/// <param name="power">    ダメージ量</param>
	/// <param name="type">      キャラクタータイプ</param>
	void ShootBullet(
		const Vector3& weaponPos,
		const Vector3& targetPos,
		const Vector3& targetVel,
		float speed,float power,
		CharacterType type);
	void ShootBullet(
		const Vector3& weaponPos,
		const Vector3& direction,
		float speed,
		float power,
		CharacterType type);
	
	/// <summary>
	/// ミサイルの発射処理
	/// </summary>
	/// <param name="ownerWeapon"> 所有武器</param>
	/// <param name="speed">       ミサイルの速度</param>
	/// <param name="power">      ダメージ量</param>
	/// <param name="type">        キャラクタータイプ</param>
	void ShootMissile(BaseWeapon* ownerWeapon,VerticalMissileInfo vmInfo, float speed,float power, CharacterType type);

	void ShootBazookaBullet(
		const Vector3& weaponPos,
		const Vector3& targetPos,
		const Vector3& targetVel,
		float speed,float power,
		CharacterType type);
	void ShootBazookaBullet(
		const Vector3& weaponPos,
		const Vector3& direction,
		float speed,
		float power,
		CharacterType type);

	//----- getter ---------------------------
	
	//全ての弾を取得
	std::vector<Bullet*> GetAllPlayerBullets();
	std::vector<Bullet*> GetAllEnemyBullets();
	//全てのミサイルを取得
	std::vector<VerticalMissile*> GetAllPlayerMissiles();
	std::vector<VerticalMissile*> GetAllEnemyMissiles();
	//全てのバズーカ弾を取得
	std::vector<Bullet*> GetAllPlayerBazookaBullets();
	std::vector<Bullet*> GetAllEnemyBazookaBullets();

private:

	Object3dCommon* object3dCommon_ = nullptr; // 3Dオブジェクト共通クラス

	std::string bulletFilePath_;               // 弾のファイルパス
	std::string missileFilePath_;              // ミサイルのファイルパス
	std::string bazookaBulletFilePath_;        // バズーカ弾のファイルパス

	std::unique_ptr<BulletPool> playerBulletPool_;   // 弾プール
	std::unique_ptr<BulletPool> enemyBulletPool_;   // 弾プール
	std::unique_ptr<MissilePool> playerMissilePool_; // ミサイルプール
	std::unique_ptr<MissilePool> enemyMissilePool_; // ミサイルプール
	std::unique_ptr<BulletPool> playerBazookaBulletPool_; // バズーカ弾プール
	std::unique_ptr<BulletPool> enemyBazookaBulletPool_; // バズーカ弾プール

	BulletEffectConfig playerBulletEffectConfig_{};
	BulletEffectConfig enemyBulletEffectConfig_{};
	BulletEffectConfig playerMissileEffectConfig_{};
	BulletEffectConfig enemyMissileEffectConfig_{};
	BulletEffectConfig playerBazookaBulletEffectConfig_{};
	BulletEffectConfig enemyBazookaBulletEffectConfig_{};
};