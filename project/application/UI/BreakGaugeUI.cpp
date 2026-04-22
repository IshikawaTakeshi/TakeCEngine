#include "BreakGaugeUI.h"
#include "engine/Math/Easing.h"
#include "engine/Utility/StringUtility.h"
#include "engine/base/TakeCFramework.h"
#include <algorithm>


using namespace TakeC;

//======================================================================
//	デストラクタ
//======================================================================
BreakGaugeUI::~BreakGaugeUI() {
	if (eventObserverID_[0] != -1) {
		TakeCFrameWork::GetEventManager()->RemoveObserver(
			"BreakGaugeUpdate_" + listeningEventName_, eventObserverID_[0]);
	}
	if (eventObserverID_[1] != -1) {
		TakeCFrameWork::GetEventManager()->RemoveObserver(resetEventName_,
			eventObserverID_[1]);
	}
	if (eventObserverID_[2] != -1) {
		TakeCFrameWork::GetEventManager()->RemoveObserver(
			"Initialize_BreakStunState_" + listeningEventName_,
			eventObserverID_[2]);
	}
}

//======================================================================
//	初期化
//======================================================================
void BreakGaugeUI::Initialize(TakeC::SpriteManager* spriteManager,
	const std::string& configName,
	const std::string& eventName) {
	// 親の初期化
	BaseUI::Initialize(spriteManager);

	listeningEventName_ = eventName;

	// スプライトの生成
	// (※スプライト名/ファイルパスはプロジェクトの設定に合わせる想定) （実際には
	// JsonConfig

	for (int i = 0; i < BreakGaugeType::kSize; ++i) {
		std::string fullPath =
			configName + "_" +
			StringUtility::EnumToString(static_cast<BreakGaugeType>(i)) + ".json";
		CreateAndRegisterSpriteFromJson(fullPath);
	}
	// JSONで設定されたサイズを最大幅として使用（解像度に合わせてスケーリング）
	delayGaugeMaxWidth_  = sprites_[DELAY]->GetSize().x * TakeC::WinApp::widthPercent_;
	actualGaugeMaxWidth_ = sprites_[ACTUAL]->GetSize().x * TakeC::WinApp::widthPercent_;

	defaultDelaySpriteColor_  = sprites_[DELAY]->GetMaterialColor();
	defaultActualSpriteColor_ = sprites_[ACTUAL]->GetMaterialColor();

	// イベントリスナーの登録
	// player,enemyで登録するイベント名が違う想定なので、イベント名は引数で受け取っている；
	eventObserverID_[0] = TakeCFrameWork::GetEventManager()->AddObserver(
		"BreakGaugeUpdate_" + listeningEventName_,
		[this](const std::any& data) { this->OnBreakGaugeUpdated(data); });
	// ブレイク状態復帰時のイベントも監視して、ゲージをリセットする
	resetEventName_ = "BreakGaugeUpdate_Reset_" + listeningEventName_;
	eventObserverID_[1] = TakeCFrameWork::GetEventManager()->AddObserver(
		resetEventName_, [this](const std::any& data) {
			this->OnBreakGaugeUpdated(data);
			sprites_[DELAY]->SetMaterialColor(defaultDelaySpriteColor_);
			sprites_[ACTUAL]->SetMaterialColor(defaultActualSpriteColor_);
		});
	eventObserverID_[2] = TakeCFrameWork::GetEventManager()->AddObserver(
		"Initialize_BreakStunState_" + listeningEventName_,
		[this](const std::any& data) {
			data;
			sprites_[DELAY]->SetMaterialColor(
				{ 1.0f, 0.0f, 0.1f, 1.0f }); // 赤色っぽいピンク色
			sprites_[ACTUAL]->SetMaterialColor(
				{ 1.0f, 0.0f, 0.1f, 1.0f }); // 赤色っぽいピンク色
		});

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
	if (sprites_[DELAY]) {
		Vector2 size_Delay = sprites_[DELAY]->GetSize();
		size_Delay.x = delayGaugeMaxWidth_ * currentDelayRatio_;
		sprites_[DELAY]->SetSize(size_Delay);
	}
	if (sprites_[ACTUAL]) {
		Vector2 size_Actual = sprites_[ACTUAL]->GetSize();
		size_Actual.x = actualGaugeMaxWidth_ * currentActualRatio_;
		sprites_[ACTUAL]->SetSize(size_Actual);
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

	sprites_[FLAME]->SetTranslate(position);
	sprites_[DELAY]->SetTranslate(position);
	sprites_[ACTUAL]->SetTranslate(position);
}

//======================================================================
//	EventManagerからのコールバック
//======================================================================
void BreakGaugeUI::OnBreakGaugeUpdated(const std::any& data) {
	// BreakGaugeInfo のポインタとして受け取る想定
	if (BreakGaugeInfo* const* ptrInfo = std::any_cast<BreakGaugeInfo*>(&data)) {

		const BreakGaugeInfo* info = *ptrInfo;

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

		// 実際の削れ値 + これから減衰される分
		float delayValue = actualValue + delaySum;
		targetDelayRatio_ = std::clamp(delayValue / maxG, 0.0f, 1.0f);
	}
}
