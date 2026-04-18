#include "PauseMenuUI.h"
#include "engine/Base/SpriteManager.h"
#include "engine/Base/TakeCFrameWork.h"

using namespace TakeC;

//============================================================================
//	初期化
//============================================================================
void PauseMenuUI::Initialize(SpriteManager* spriteManager) {
    BaseUI::Initialize(spriteManager);

    // UIConfigの一括読み込み
    LoadUIConfig("PauseMenu");

    // 各スプライトポインタの取得
    bg_ = GetSprite("BG");
    cursor_ = GetSprite("Cursor");

    itemSprites_.clear();
    for (int i = 0; i < (int)itemNames_.size(); ++i) {
        Sprite* item = GetSprite("Item" + std::to_string(i));
        itemSprites_.push_back(item);
    }

    SetActive(false);
    isOpen_ = false;
    isClosing_ = false;
    result_ = PauseMenuResult::None;
    currentIndex_ = 0;
    UpdateVisual();
}

//============================================================================
//	更新処理
//============================================================================
void PauseMenuUI::Update() {
    if (!IsActive() || !isOpen_) return;

    // 終了進行中でない場合のみ入力を受け付ける
    if (!isClosing_) {
        UpdateInput();
    }
    UpdateVisual();
}

//============================================================================
// ポーズメニューの開閉処理
//============================================================================
void PauseMenuUI::Open() {
    isOpen_ = true;
    isClosing_ = false;
    SetActive(true);
    result_ = PauseMenuResult::None;
    currentIndex_ = 0;

    //BGの色を変化させる
	bg_->Animation()->PlayFade(0.0f, 1.0f, 0.1f, 0.0f, Easing::EasingType::LINEAR, SpriteAnimator::PlayMode::ONCE);

    UpdateVisual();
}

void PauseMenuUI::Close() {
    if (isClosing_) return;
    isClosing_ = true;
   
    //BGの色を変化させる
    bg_->Animation()->PlayFade(1.0f, 0.0f, 0.1f, 0.0f, Easing::EasingType::LINEAR, SpriteAnimator::PlayMode::ONCE);
	//ポストエフェクトを再生
    TakeCFrameWork::GetPostEffectManager()->PlayEffect("Outline_FadeIn");
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
    // ポーズメニューUIを閉じる
    Close();
}

//============================================================================
//	ビジュアル更新処理
//============================================================================
void PauseMenuUI::UpdateVisual() {
    if (cursor_ && currentIndex_ < (int)itemSprites_.size() && itemSprites_[currentIndex_]) {
        Vector2 itemPos = itemSprites_[currentIndex_]->GetTranslate();
        // カーソルを項目の右側に配置
        Vector2 cp = { itemPos.x + 140.0f, itemPos.y };
        cursor_->SetTranslate(cp);
    }

    // 選択中だけ明るくする
    for (int i = 0; i < (int)itemSprites_.size(); ++i) {
        if (!itemSprites_[i]) continue;
        Vector4 color = (i == currentIndex_)
            ? Vector4{1, 1, 1, 1}
		: PauseColor_;
        itemSprites_[i]->SetMaterialColor(color);
    }

    // isOpen_ の更新は ConsumeResult もしくは Update 継続のためにここでは調整しないか、
    // アニメーション終了判定を共通化する
    if (bg_->Animation()->IsFinished() && !isClosing_) {
        // 通常のオープンアニメーション終了時などはここを通る可能性がある
    }
}

//============================================================================
//	位置設定
//============================================================================
void PauseMenuUI::SetPosition(const Vector2& position) {
    BaseUI::SetPosition(position);

    // 背景の位置を更新
    if (bg_) {
        bg_->SetTranslate(position);
    }
    UpdateVisual();
}

//============================================================================
//	結果の消費
//============================================================================
PauseMenuResult PauseMenuUI::ConsumeResult() {
    // 終了処理中で、かつアニメーションが完了している場合のみ結果を返す
    if (isClosing_) {
        if (bg_->Animation()->IsFinished()) {
            PauseMenuResult r = result_;
            result_ = PauseMenuResult::None;
            isClosing_ = false;
            isOpen_ = false;
            SetActive(false);
            return r;
        }
        return PauseMenuResult::None;
    }

    // 終了処理中でない場合は即座に返すが、基本的にClose()を経由するので通常はNoneが返る
    PauseMenuResult r = result_;
    result_ = PauseMenuResult::None;
    return r;
}
