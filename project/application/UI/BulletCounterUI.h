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

	void Initialize(SpriteCommon* spriteCommon,const Vector2& position);

	void Update();

	void Draw();

	void UpdateImGui([[maybe_unused]] const std::string& name);

	uint32_t GetBulletCount() const;
	uint32_t GetRemainingBulletCount() const;

	const Vector2& GetBulletCounterPosition() const;

	void SetBulletCount(uint32_t count);
	void SetRemainingBulletCount(uint32_t count);
	void SetReloadingState(bool isReloading);
	void SetBulletCounterPosition(const Vector2& position);
	void SetWeaponIconUV(int weaponIndex);

private:

	void SetDigitUV(Sprite* sprite, int digit);

private:

	std::vector<std::unique_ptr<Sprite>> bulletCounterSprite_;    // 弾数カウンターのスプライト
	std::vector<std::unique_ptr<Sprite>> maxBulletCounterSprite_; // 最大弾数カウンターのスプライト
	std::vector<std::unique_ptr<Sprite>> WeaponPositionSprite_;   // 武器の位置を示すスプライト
	std::unique_ptr<Sprite> reloadSprite_; // リロード中のスプライト
	std::unique_ptr<Sprite> separatorSprite_; // スプライトの間のセパレーター
	std::unique_ptr<Sprite> weaponIconSprite_; // 武器アイコンのスプライト
	uint32_t bulletCount_ = 0;             // 現在の弾数
	uint32_t remainingBulletCount_ = 0; // 残りの弾数

	std::vector<int> digits_;
	std::vector<int> maxBulletDigits_;
	Vector2 bulletCounterPos_ = { 50.0f, 100.0f }; // スプライトの位置
	Vector2 counterSpriteSize_ = { 28.0f, 28.0f }; // スプライトのサイズ
	Vector2 reloadSpriteSize_ = { 50.0f, 32.0f };  // リロードスプライトのサイズ
	Vector2 separatorSpriteSize_ = { 5.0f, 28.0f }; // セパレータースプライトのサイズ

	bool isReloading_ = false; // リロード中かどうかのフラグ
	//スプライトの間隔
	float spriteSpace_ = 5.0f; // スプライト間の間隔
};

