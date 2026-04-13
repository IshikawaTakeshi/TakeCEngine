#pragma once
#include "engine/2d/BaseUI.h"
#include "application/Provider/PauseMenuPadInput.h"
#include <vector>
#include <string>

//============================================================================
// ポーズメニューの選択結果
//============================================================================
enum class PauseMenuResult {
    None,
    Resume,
    Retry,
    ToTitle
};

//============================================================================
// PauseMenuUI class
//============================================================================
class PauseMenuUI : public BaseUI {
public:

	//=============================================================================
	// functions
	//=============================================================================

    /// <summary>
	/// コンストラクタ・デストラクタ
    /// </summary>
    PauseMenuUI() = default;
    ~PauseMenuUI() override = default;

    /// <summary>
	/// 初期化
    /// </summary>
    /// <param name="spriteManager"></param>
    void Initialize(TakeC::SpriteManager* spriteManager);

    /// <summary>
	/// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
	/// 描画処理 (基本的には何もしない。SpriteManager側で一括描画されるため)
    /// </summary>
    void Draw() override {}

	// UIの基準座標を設定
    void SetPosition(const Vector2& position) override;

    /// <summary>
	/// ポーズメニューの開閉
    /// </summary>
    void Open();
    void Close();
    bool IsOpen() const { return isOpen_; }

    // 1フレームだけ結果を受け取る
    PauseMenuResult ConsumeResult(); 

private:

    /// <summary>
	/// 入力の更新と処理
    /// </summary>
    void UpdateInput();

    /// <summary>
	/// ビジュアルの更新 (カーソル位置の更新など)
    /// </summary>
    void UpdateVisual();

    /// <summary>
	/// カーソルの移動
    /// </summary>
    /// <param name="delta"></param>
    void MoveCursor(int delta);

    /// <summary>
	/// 現在の選択肢の実行
    /// </summary>
    void ExecuteCurrent();

private:

	// 入力デバイス
    PauseMenuPadInput input_;
	// メニュー項目
    std::vector<std::string> itemNames_ = { "Resume", "Retry", "Title" };
	// 現在の選択インデックス
    int currentIndex_ = 0;
	// メニューが開いているか
    bool isOpen_ = false;
	// メニューの選択結果
    PauseMenuResult result_ = PauseMenuResult::None;

    // sprites
    Sprite* bg_ = nullptr;
    Sprite* cursor_ = nullptr;
    std::vector<Sprite*> itemSprites_;
	Vector4 PauseColor_ = { 0.3f, 0.3f, 0.3f, 1.0f };

};