#pragma once
#include "Weapon/Bullet/Bullet.h"
#include <cstdint>
#include <vector>
#include <memory>

//============================================================================
// BulletPool class
//============================================================================
class BulletPool {
public:

	BulletPool() = default;
	~BulletPool() = default;

	//初期化
	void Initialize(size_t size);
	//開放処理
	void Finalize();
	//弾の取得
	Bullet* GetBullet();
	//弾の更新
	void UpdateAllBullet();
	//弾の描画
	void DrawAllBullet();
	//弾のコライダー描画
	void DrawAllCollider();

	//プールの取得
	std::vector<Bullet*> GetPool();

private:

	//弾のプール
	std::vector<std::unique_ptr<Bullet>> pool_;
};