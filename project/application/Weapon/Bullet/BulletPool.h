#pragma once
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/Bullet/ObjectPool.h"
#include <cstdint>
#include <vector>
#include <memory>

//============================================================================
// BulletPool class
//============================================================================
class BulletPool : public ObjectPool<Bullet> {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BulletPool() = default;
	~BulletPool() = default;

protected:

	//型ごとの初期化処理
	void OnInitializeObject(Bullet& object,const std::string& modelFilePath,const BulletEffectConfig& effectConfig) override;

};