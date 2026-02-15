#pragma once
#include "engine/Math/Vector2.h"
#include "engine/Math/Vector4.h"
#include "engine/Base/SpriteManager.h"
#include "engine/2d/Sprite.h"
#include <memory>
#include <vector>
#include <string>

//============================================================================
// BaseUI Class
// 
// 全てのUIの親となるクラス。
// 基本的な描画、更新、表示切り替え機能を提供する。
// SpriteManagerを用いてスプライトを一括管理する。
//============================================================================
class BaseUI {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BaseUI() = default;
	virtual ~BaseUI() = default;

	//=====================================================
	// functions
	//=====================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="spriteManager">外部で管理されているSpriteManagerへのポインタ</param>
	virtual void Initialize(TakeC::SpriteManager* spriteManager);

	/// <summary>
	/// 更新処理 (UI独自のロジック更新のみ)
	/// </summary>
	virtual void Update();

	/// <summary>
	/// 描画処理 (基本的には何もしない。SpriteManager側で一括描画されるため)
	/// ※独自の描画処理が必要な場合のみオーバーライド
	/// </summary>
	virtual void Draw();

	/// <summary>
	/// ImGui更新
	/// </summary>
	virtual void UpdateImGui(const std::string& name);

	
	// スプライトを追加するヘルパー関数
	Sprite* CreateAndRegisterSprite(const std::string& textureName, const Vector2& position = {0.0f, 0.0f});

	// JSONからスプライトを生成・登録するヘルパー関数
	Sprite* CreateAndRegisterSpriteFromJson(const std::string& jsonFilePath);


	//=====================================================
	// accessors
	//=====================================================

	// 名前でスプライトを取得
	Sprite* GetSprite(const std::string& name);

	// 表示・非表示
	void SetActive(bool isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }

	// 基準座標の設定
	virtual void SetPosition(const Vector2& position) { position_ = position; }
	const Vector2& GetPosition() const { return position_; }

	// 全体の透明度設定（フェード用）
	void SetAlpha(float alpha);

protected:

	//=====================================================
	// variables
	//=====================================================
	

	// 外部のスプライト管理マネージャー（所有権は持たない）
	TakeC::SpriteManager* spriteManager_ = nullptr;

	std::vector<Sprite*> sprites_;

	Vector2 position_ = { 0.0f, 0.0f };
	bool isActive_ = true;
	float alpha_ = 1.0f;
};
