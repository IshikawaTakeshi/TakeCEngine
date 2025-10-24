#pragma once
#include "engine/math/Vector2.h"
#include <memory>
#include <string>
#include <cstdint>
#include <vector>

// 前方宣言
class SpriteCommon;
class Sprite;

//============================================================================
// BulletCounterUI class
//============================================================================
class BulletCounterUI {
public:

	BulletCounterUI() = default;
	~BulletCounterUI() = default;

	//========================================================================
	// functions
	//========================================================================

	// 初期化
	void Initialize(SpriteCommon* spriteCommon,const Vector2& position);
	// 更新
	void Update();
	// 描画
	void Draw();
	// ImGuiの更新
	void UpdateImGui([[maybe_unused]] const std::string& name);


	//----- getter ---------------------------

	// 弾数取得
	uint32_t GetBulletCount() const;
	// 残り弾数取得
	uint32_t GetRemainingBulletCount() const;
	// 弾数カウンターの位置取得
	const Vector2& GetBulletCounterPosition() const;

	//----- setter ---------------------------

	// 弾数設定
	void SetBulletCount(uint32_t count);
	// 残り弾数設定
	void SetRemainingBulletCount(uint32_t count);
	// リロード状態設定
	void SetReloadingState(bool isReloading);
	// 弾数カウンターの位置設定
	void SetBulletCounterPosition(const Vector2& position);
	// 武器アイコンのUV設定
	void SetWeaponIconUV(int weaponIndex);

private:

	// 桁ごとのUV設定
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