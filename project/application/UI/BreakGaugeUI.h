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

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BreakGaugeUI() = default;
	~BreakGaugeUI() override;

	enum BreakGaugeType {
		FLAME, // ゲージの背景（フレームなど）
		DELAY,  // 遅延減衰待ちの蓄積値
		ACTUAL, // 実数のブレイクゲージ
		kSize // ゲージの種類数
	};

	//==================================================================
	// functions
	//==================================================================



	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="spriteManager">SpriteManagerへのポインタ</param>
	/// <param name="eventName">監視対象のイベント名（例:
	/// "PlayerBreakGaugeUpdate"）</param>
	void Initialize(TakeC::SpriteManager* spriteManager,
		const std::string& configName,
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
	std::array<uint32_t, 3> eventObserverID_{};
	std::string listeningEventName_;
	//ブレイク状態復帰時のイベント名
	std::string resetEventName_;

	// ゲージのデフォルト色（JSONで設定された色を保持しておいて、ブレイクスタン状態から復帰したときに戻すため）
	Vector4 defaultDelaySpriteColor_;
	Vector4 defaultActualSpriteColor_;
	// ブレイクスタン状態の色（赤色っぽいピンク色）
	Vector4 breakStunSpriteColor_ = { 1.0f, 0.0f, 0.1f, 1.0f };

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
