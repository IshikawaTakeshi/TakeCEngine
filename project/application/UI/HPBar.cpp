#include "HPBar.h"
#include "engine/2d/Sprite.h"
#include "engine/Math/Easing.h"
#include "engine/Utility/StringUtility.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"


using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void HPBar::Initialize(TakeC::SpriteManager* spriteManager,
	const std::string& configName) {
	// BaseUIの初期化
	BaseUI::Initialize(spriteManager);

	// 各コンポーネントのJSONを読み込み
	// BACKGROUND, DAMAGE, FOREGROUND, OWNER の各サフィックスを想定
	for (int i = 0; i < kTotalComponents; ++i) {
		std::string componentSuffix = "";
		switch (i) {
		case BACKGROUND:
			componentSuffix = "_BACKGROUND";
			break;
		case DAMAGE:
			componentSuffix = "_DAMAGE";
			break;
		case FOREGROUND:
			componentSuffix = "_FOREGROUND";
			break;
		case OWNER:
			componentSuffix = "_OWNER";
			break;
		}

		std::string fullPath = configName + componentSuffix + ".json";
		Sprite* s = CreateAndRegisterSpriteFromJson(fullPath);

		// 個別のポインタを保持
		switch (i) {
		case BACKGROUND:
			backgroundSprite_ = s;
			break;
		case DAMAGE:
			damageBarSprite_ = s;
			break;
		case FOREGROUND:
			foregroundSprite_ = s;
			break;
		case OWNER:
			ownerNameSprite_ = s;
			break;
		}
	}

	// 初期サイズの保存
	if (backgroundSprite_)
		backgroundMaxWidth_ = backgroundSprite_->GetSize().x;
	if (foregroundSprite_)
		foregroundMaxWidth_ = foregroundSprite_->GetSize().x;
	if (damageBarSprite_)
		damageBarMaxWidth_ = damageBarSprite_->GetSize().x;

	margin_ = 2.0f; // 枠の太さ

	damageDelayTimer_.Initialize(1.5f,
		0.0f); // ダメージバーの遅延タイマーを初期化
}

//===================================================================================
// HP情報の更新
//===================================================================================
void HPBar::SetHP(float currentHP, float maxHP) {
	currentHP = std::max(0.0f, std::min(currentHP, maxHP));
	maxHP_ = maxHP;

	if (currentHP < currentHP_) {
		// ダメージを受けた場合、遅延タイマーをリセット
		damageDelayTimer_.Initialize(1.5f, 0.0f);
	}

	currentHP_ = currentHP;
	hpRatio_ = (maxHP_ > 0.0f) ? (currentHP_ / maxHP_) : 0.0f;
}

//===================================================================================
// 更新
//===================================================================================
void HPBar::Update() {
	if (!isActive_) {
		alpha_ -= 1.0f * TakeCFrameWork::GetDeltaTime();
		if (alpha_ <= 0.0f) {
			alpha_ = 0.0f;
			return;
		}
	} else {
		alpha_ = 1.0f;
	}

	// アルファ値の反映 (BaseUIの機能を利用)
	SetAlpha(alpha_);

	// ダメージバーの遅延タイマーを更新
	damageDelayTimer_.Update();

	// ダメージバーの補間処理
	if (damageDelayTimer_.IsFinished()) {
		damageBarRatio_ =
			Easing::Lerp(damageBarRatio_, hpRatio_,
				damageLerpSpeed_ * TakeCFrameWork::GetDeltaTime());
	}

	// ゲージの長さを更新
	if (foregroundSprite_) {
		Vector2 size = foregroundSprite_->GetSize();
		size.x = foregroundMaxWidth_ * hpRatio_;
		foregroundSprite_->SetSize(size);
	}

	if (damageBarSprite_) {
		Vector2 size = damageBarSprite_->GetSize();
		size.x = damageBarMaxWidth_ * damageBarRatio_;
		damageBarSprite_->SetSize(size);
	}

	// スプライトの更新はSpriteManagerが行うためここでは不要だが、
	// BaseUIを通じた基本的な更新が必要な場合は呼ぶ
	BaseUI::Update();
}

//===================================================================================
// 描画
//===================================================================================
void HPBar::Draw() {
	if (!isActive_ || alpha_ <= 0.0f)
		return;
	// 描画は登録順で行われるが、明示的な順序が必要な場合はここでDrawを呼ぶ
	// 現状はSpriteManagerが一括で描画する設計を尊重
}

//===================================================================================
// ImGuiの更新
//===================================================================================
void HPBar::UpdateImGui(const std::string& name) {
	BaseUI::UpdateImGui(name);

#if defined(_DEBUG) || defined(_DEVELOP)
	if (ImGui::TreeNode((name + " HPBar").c_str())) {
		ImGui::ProgressBar(hpRatio_, ImVec2(0.0f, 0.0f), "Current HP");
		ImGui::ProgressBar(damageBarRatio_, ImVec2(0.0f, 0.0f), "Damage Bar");
		ImGui::TreePop();
	}
#endif
}

//===================================================================================
// 位置を設定
//===================================================================================
void HPBar::SetPosition(const Vector2& position) {
	BaseUI::SetPosition(position);

	if (backgroundSprite_)
		backgroundSprite_->SetTranslate(position);
	if (foregroundSprite_)
		foregroundSprite_->SetTranslate(position + Vector2{ margin_, margin_ });
	if (damageBarSprite_)
		damageBarSprite_->SetTranslate(position + Vector2{ margin_, margin_ });

	// OwnerNameはJSONでの初期位置をベースにするか、positionからの相対にする
	// 現状はSetPositionで一律固定の位置にくるように調整が必要。
	// ここでは背景からの相対オフセットを固定で持たせる例
	if (ownerNameSprite_) {
		Vector2 ownerOffset = { 1.0f, -30.0f }; // 適宜調整
		ownerNameSprite_->SetTranslate(position + ownerOffset);
	}
}
