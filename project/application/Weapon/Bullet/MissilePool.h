#pragma once
#include "application/Weapon/Bullet/VerticalMissile.h"
#include <cstdint>
#include <vector>
#include <memory>

//============================================================================
// MissilePool class
//============================================================================
class MissilePool {
public:
	MissilePool() = default;
	~MissilePool() = default;

	//========================================================================
	// function
	//========================================================================

	//初期化
	void Initialize(size_t size);
	//終了処理
	void Finalize();
	//ミサイルの取得
	VerticalMissile* GetMissile();
	//ミサイルの更新
	void UpdateAllMissiles();
	//ミサイルの描画
	void DrawAllMissiles();
	//ミサイルのコライダー描画
	void DrawAllCollider();

	//プールの取得
	std::vector<VerticalMissile*> GetPool();

private:

	//ミサイルプール
	std::vector<std::unique_ptr<VerticalMissile>> pool_;
};