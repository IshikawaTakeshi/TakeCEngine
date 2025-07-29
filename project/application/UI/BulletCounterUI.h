#pragma once
#include "engine/math/Vector2.h"
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

class SpriteCommon;
class Sprite;
class BulletCounterUI {
public:

	BulletCounterUI() = default;
	~BulletCounterUI() = default;

	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

	void UpdateImGui();

	uint32_t GetBulletCount() const;
	uint32_t GetMaxBulletCount() const;

	void SetBulletCount(uint32_t count);
	void SetMaxBulletCount(uint32_t count);
	void SetReloadingState(bool isReloading);
	void SetReloadTimer(float timer);

private:

	std::vector<std::unique_ptr<Sprite>> bulletCounterSprite_; // 弾数カウンターのスプライト
	std::vector<std::unique_ptr<Sprite>> WeaponPositionSprite_; // 武器の位置を示すスプライト
	std::unique_ptr<Sprite> reloadSprite_; // リロード中のスプライト
	uint32_t bulletCount_ = 0; // 現在の弾数
	uint32_t maxBulletCount_ = 100; // 最大弾数

	bool isReloading_ = false; // リロード中かどうかのフラグ
	float reloadTimer_ = 0.0f; // リロードタイマー
	//スプライトの間隔
	float spriteSpace_ = 5.0f; // スプライト間の間隔
	Vector2 bulletCounterPos_ = { 50.0f, 100.0f }; // スプライトの位置
};

