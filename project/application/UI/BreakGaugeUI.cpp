#include "BreakGaugeUI.h"
#include "engine/Math/Easing.h"
#include "engine/base/TakeCFramework.h"
#include <algorithm>

using namespace TakeC;

//======================================================================
//	デストラクタ
//======================================================================
BreakGaugeUI::~BreakGaugeUI() {
	if (eventObserverID_ != -1) {
		TakeCFrameWork::GetEventManager()->RemoveObserver(listeningEventName_,
			eventObserverID_);
	}
}

//======================================================================
//	初期化
//======================================================================
void BreakGaugeUI::Initialize(TakeC::SpriteManager* spriteManager,
	const std::string& eventName) {
	// 親の初期化
	BaseUI::Initialize(spriteManager);

	listeningEventName_ = eventName;

	// スプライトの生成
	// (※スプライト名/ファイルパスはプロジェクトの設定に合わせる想定) （実際には
	// JsonConfig
	// で指定するなどの形でも良いですが、ここでは直接テクスチャを指定します。
	// 　必要に応じて "UI/BreakGauge_Frame.png"
	// 等のファイルパスを追加/修正してください）

	// TODO: 実際のテクスチャパスに置き換えるか、ダミーの白テクスチャを利用する
	frameSprite_ = CreateAndRegisterSprite("white1x1.png");
	delayGaugeSprite_ = CreateAndRegisterSprite("white1x1.png");
	actualGaugeSprite_ = CreateAndRegisterSprite("white1x1.png");

	// スプライトの色やサイズを設定
	if (frameSprite_) {
		frameSprite_->SetSize({ 300.0f, 20.0f });
		frameSprite_->SetMaterialColor({ 0.2f, 0.2f, 0.2f, 1.0f }); // ダークグレー枠
		frameSprite_->SetAnchorPoint({ 0.0f, 0.5f });               // 左端基準
	}

	if (delayGaugeSprite_) {
		delayGaugeSprite_->SetSize({ 296.0f, 16.0f });
		delayGaugeSprite_->SetMaterialColor(
			{ 1.0f, 1.0f, 0.0f, 1.0f });                   // 黄色ゲージ（遅延分）
		delayGaugeSprite_->SetAnchorPoint({ 0.0f, 0.5f }); // 左端基準
		delayGaugeMaxWidth_ = delayGaugeSprite_->GetSize().x;
	}

	if (actualGaugeSprite_) {
		actualGaugeSprite_->SetSize({ 296.0f, 16.0f });
		actualGaugeSprite_->SetMaterialColor(
			{ 1.0f, 0.0f, 0.0f, 1.0f });                    // 赤色ゲージ（実数）
		actualGaugeSprite_->SetAnchorPoint({ 0.0f, 0.5f }); // 左端基準
		actualGaugeMaxWidth_ = actualGaugeSprite_->GetSize().x;
	}

	// 初期位置のリセット
	SetPosition(position_);

	// イベントリスナーの登録
	eventObserverID_ = TakeCFrameWork::GetEventManager()->AddObserver(
		eventName,
		[this](const std::any& data) { this->OnBreakGaugeUpdated(data); });

	// 初期値は0
	targetDelayRatio_ = 0.0f;
	targetActualRatio_ = 0.0f;
	currentDelayRatio_ = 0.0f;
	currentActualRatio_ = 0.0f;
}

//======================================================================
//	更新処理
//======================================================================
void BreakGaugeUI::Update() {
	BaseUI::Update();
	if (!isActive_)
		return;

	// ゲージの見た目（補間）更新
	float deltaTime = TakeCFrameWork::GetDeltaTime();

	// 黄ゲージ（遅延）は少しゆっくり赤に追いつくか、実数に合わせて減衰
	// 赤ゲージ（実数）は即座に反映するか、少しだけ補間
	currentDelayRatio_ =
		Easing::Lerp(currentDelayRatio_, targetDelayRatio_, 10.0f * deltaTime);
	currentActualRatio_ =
		Easing::Lerp(currentActualRatio_, targetActualRatio_, 15.0f * deltaTime);

	// スプライトサイズの更新 (左端Anchorを前提にX軸スケールを変更)
	if (delayGaugeSprite_) {
		Vector2 size = delayGaugeSprite_->GetSize();
		size.x = delayGaugeMaxWidth_ * currentDelayRatio_;
		delayGaugeSprite_->SetSize(size);
	}

	if (actualGaugeSprite_) {
		Vector2 size = actualGaugeSprite_->GetSize();
		size.x = actualGaugeMaxWidth_ * currentActualRatio_;
		actualGaugeSprite_->SetSize(size);
	}
}

//======================================================================
//	描画処理
//======================================================================
void BreakGaugeUI::Draw() {
	BaseUI::Draw();
	// SpriteManager経由の一括描画を利用する場合はそのまま
	// （※BaseUIのデフォルトと同じく中身は空またはSprite別個描画）
}

//======================================================================
//	ImGui更新
//======================================================================
void BreakGaugeUI::UpdateImGui(const std::string& name) {
	BaseUI::UpdateImGui(name);

#if defined(_DEBUG) || defined(_DEVELOP)
	if (ImGui::TreeNode((name + " Debug").c_str())) {
		ImGui::Text("Event: %s", listeningEventName_.c_str());
		ImGui::ProgressBar(currentActualRatio_, ImVec2(0.0f, 0.0f), "Actual");
		ImGui::ProgressBar(currentDelayRatio_, ImVec2(0.0f, 0.0f), "Delay");
		ImGui::TreePop();
	}
#endif
}

//======================================================================
//	座標更新
//======================================================================
void BreakGaugeUI::SetPosition(const Vector2& position) {
	BaseUI::SetPosition(position);

	if (frameSprite_) {
		frameSprite_->SetTranslate(position_);
	}

	// 枠の分少しオフセット
	Vector2 innerPos = position_ + Vector2{ 2.0f, 0.0f };
	if (delayGaugeSprite_) {
		delayGaugeSprite_->SetTranslate(innerPos);
	}
	if (actualGaugeSprite_) {
		actualGaugeSprite_->SetTranslate(innerPos);
	}
}

//======================================================================
//	EventManagerからのコールバック
//======================================================================
void BreakGaugeUI::OnBreakGaugeUpdated(const std::any& data) {
	// BreakGaugeInfo のポインタとして受け取る想定
	if (const BreakGaugeInfo* info = std::any_cast<BreakGaugeInfo>(&data)) {

		float maxG = info->maxBreakGauge;
		if (maxG <= 0.0f) {
			targetDelayRatio_ = 0.0f;
			targetActualRatio_ = 0.0f;
			return;
		}

		// 実数値の割合
		float actualValue = info->breakGauge;
		targetActualRatio_ = std::clamp(actualValue / maxG, 0.0f, 1.0f);

		// 遅延待ち（まだ減衰が始まっていない蓄積値）の合計を計算
		float delaySum = 0.0f;
		for (const auto& entry : info->entries) {
			delaySum += entry.amount;
		}

		// 黄色ゲージ＝実際の削れ値 + これから減衰される分
		float delayValue = actualValue + delaySum;
		targetDelayRatio_ = std::clamp(delayValue / maxG, 0.0f, 1.0f);
	}
}
