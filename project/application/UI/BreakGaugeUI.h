#pragma once
#include "application/Entity/GameCharacterInfo.h"
#include "engine/2d/BaseUI.h"
#include "engine/Math/Vector2.h"
#include <any>
#include <string>

//============================================================================
// BreakGaugeUI Class
//
// 対象のBreakGauge値をEventManager経由で監視し、UI上のゲージとして表示する。
// 「実数（赤）」と「遅延減衰待ちの蓄積値（黄）」の二重ゲージ構造を持つ。
//============================================================================
class BreakGaugeUI : public BaseUI {
public:
	BreakGaugeUI() = default;
	~BreakGaugeUI() override;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="spriteManager">SpriteManagerへのポインタ</param>
	/// <param name="eventName">監視対象のイベント名（例:
	/// "PlayerBreakGaugeUpdate"）</param>
	void Initialize(TakeC::SpriteManager* spriteManager,
		const std::string& eventName);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// ImGui更新
	/// </summary>
	void UpdateImGui(const std::string& name) override;

	/// <summary>
	/// 基準座標の設定（オーバーライドして各スプライトの座標も追従）
	/// </summary>
	void SetPosition(const Vector2& position) override;

private:
	/// <summary>
	/// EventManagerからのデータ受信コールバック
	/// </summary>
	void OnBreakGaugeUpdated(const std::any& data);

private:
	// イベント監視用ID
	int eventObserverID_ = -1;
	std::string listeningEventName_;

	// UI要素
	Sprite* frameSprite_ = nullptr;       // 外枠
	Sprite* delayGaugeSprite_ = nullptr;  // 遅延分を含むゲージ（黄など）
	Sprite* actualGaugeSprite_ = nullptr; // 実数ゲージ（赤など）

	// 各ゲージの最大描画サイズ(X軸)
	float delayGaugeMaxWidth_ = 0.0f;
	float actualGaugeMaxWidth_ = 0.0f;

	// 現在の目標比率 (0.0f ~ 1.0f)
	float targetDelayRatio_ = 0.0f;
	float targetActualRatio_ = 0.0f;

	// 補間用の現在比率
	float currentDelayRatio_ = 0.0f;
	float currentActualRatio_ = 0.0f;
};
