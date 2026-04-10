#include "PauseMenuUI.h"
#include "engine/Base/SpriteManager.h"

using namespace TakeC;

//============================================================================
//	初期化
//============================================================================
void PauseMenuUI::Initialize(SpriteManager* spriteManager) {
    BaseUI::Initialize(spriteManager);

    // 例: 適宜あなたのアセット名に合わせて変更
    bg_ = CreateAndRegisterSprite("pause_bg", basePos_);
    cursor_ = CreateAndRegisterSprite("pause_cursor", { basePos_.x - 140.0f, basePos_.y });

    itemSprites_.clear();
    for (int i = 0; i < (int)itemNames_.size(); ++i) {
        Vector2 p = { basePos_.x, basePos_.y + i * itemIntervalY_ };
        // 例: "pause_item_resume" のようなテクスチャ名にしてもOK
        Sprite* item = CreateAndRegisterSprite("pause_item_" + std::to_string(i), p);
        itemSprites_.push_back(item);
    }

    SetActive(false);
    isOpen_ = false;
    result_ = PauseMenuResult::None;
    currentIndex_ = 0;
    UpdateVisual();
}

//============================================================================
//	更新処理
//============================================================================
void PauseMenuUI::Update() {
    if (!IsActive() || !isOpen_) return;

    UpdateInput();
    UpdateVisual();
}

//============================================================================
// ポーズメニューの開閉処理
//============================================================================
void PauseMenuUI::Open() {
    isOpen_ = true;
    SetActive(true);
    result_ = PauseMenuResult::None;
    currentIndex_ = 0;
    UpdateVisual();
}

void PauseMenuUI::Close() {
    isOpen_ = false;
    SetActive(false);
}

//============================================================================
//	入力処理
//============================================================================
void PauseMenuUI::UpdateInput() {
    if (input_.IsTriggered(PauseMenuInput::Up)) {
        MoveCursor(-1);
    }
    if (input_.IsTriggered(PauseMenuInput::Down)) {
        MoveCursor(+1);
    }

    if (input_.IsTriggered(PauseMenuInput::Select)) {
        ExecuteCurrent();
    }

    if (input_.IsTriggered(PauseMenuInput::Back)) {
        result_ = PauseMenuResult::Resume;
        Close();
    }
}

//============================================================================
//	ビジュアル更新処理
//============================================================================
void PauseMenuUI::MoveCursor(int delta) {
    const int n = (int)itemNames_.size();
    currentIndex_ = (currentIndex_ + delta + n) % n;
}

//============================================================================
//	選択肢の実行処理
//============================================================================
void PauseMenuUI::ExecuteCurrent() {
    switch (currentIndex_) {
    case 0: result_ = PauseMenuResult::Resume; break;
    case 1: result_ = PauseMenuResult::Retry; break;
    case 2: result_ = PauseMenuResult::ToTitle; break;
    default: result_ = PauseMenuResult::None; break;
    }
    Close();
}

//============================================================================
//	ビジュアル更新処理
//============================================================================
void PauseMenuUI::UpdateVisual() {
    if (cursor_) {
        Vector2 cp = { basePos_.x - 140.0f, basePos_.y + currentIndex_ * itemIntervalY_ };
		cursor_->SetTranslate(cp);
    }

    // 選択中だけ明るくする例
    for (int i = 0; i < (int)itemSprites_.size(); ++i) {
        if (!itemSprites_[i]) continue;
        Vector4 color = (i == currentIndex_)
            ? Vector4{1, 1, 1, 1}
        : Vector4{0.6f, 0.6f, 0.6f, 1};
        itemSprites_[i]->SetMaterialColor(color);
    }

    // BaseUIのalpha反映も併用したい場合は SetAlpha(alpha_) を適宜
}

//============================================================================
//	位置設定
//============================================================================
void PauseMenuUI::SetPosition(const Vector2& position) {
    basePos_ = position;

	// 背景の位置を更新
    if (bg_) {
        bg_->SetTranslate(basePos_);
    }
	// アイテムの位置も更新
    for (int i = 0; i < (int)itemSprites_.size(); ++i) {
        if (itemSprites_[i]) {
            itemSprites_[i]->SetTranslate({ basePos_.x, basePos_.y + i * itemIntervalY_ });
        }
    }

	// カーソル位置も更新
    UpdateVisual();
}

//============================================================================
//	結果の消費
//============================================================================
PauseMenuResult PauseMenuUI::ConsumeResult() {
    PauseMenuResult r = result_;
    result_ = PauseMenuResult::None;
    return r;
}
