#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "engine/2d/Sprite.h"
#include "engine/math/Vector2.h"

// 前方宣言
class SpriteCommon;

//============================================================================
// SpriteManager class
//============================================================================
namespace TakeC {
	class SpriteManager {
	public:

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		SpriteManager() = default;
		~SpriteManager() = default;

		//========================================================================
		// functions
		//========================================================================

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="spriteCommon">SpriteCommonへのポインタ</param>
		void Initialize(SpriteCommon* spriteCommon);

		/// <summary>
		/// 更新処理 (全スプライト)
		/// </summary>
		void Update();

		/// <summary>
		/// 描画処理 (全スプライト)
		/// </summary>
		void Draw();

		/// <summary>
		/// ImGuiの更新
		/// </summary>
		/// <param name="groupName">ImGuiのグループ名</param>
		void UpdateImGui(const std::string& groupName);

		//-------------------------------------------------------------------------
		// 生成・登録メソッド
		//-------------------------------------------------------------------------

		/// <summary>
		/// スプライトの生成と登録 (テクスチャ指定)
		/// </summary>
		/// <param name="filePath">テクスチャのファイルパス</param>
		/// <param name="position">初期座標</param>
		/// <param name="name">登録名（省略可）。指定するとGetSpriteで取得可能になる</param>
		/// <returns>生成されたスプライトへのポインタ</returns>
		Sprite* Create(const std::string& filePath, const Vector2& position = { 0.0f, 0.0f }, const std::string& name = "");

		/// <summary>
		/// JSONファイルからスプライトを生成して登録
		/// </summary>
		/// <param name="jsonFilePath">設定JSONファイルのパス</param>
		/// <param name="name">登録名（省略可）。指定するとGetSpriteで取得可能になる</param>
		/// <returns>生成されたスプライトへのポインタ</returns>
		Sprite* CreateFromJson(const std::string& jsonFilePath);

		//-------------------------------------------------------------------------
		// 取得・操作メソッド
		//-------------------------------------------------------------------------

		/// <summary>
		/// 名前によるスプライトの取得
		/// </summary>
		/// <param name="name">登録時の名前</param>
		/// <returns>スプライトへのポインタ。見つからない場合はnullptr</returns>
		Sprite* GetSprite(const std::string& name);

		/// <summary>
		/// 全スプライトのクリア
		/// </summary>
		void Clear();

		/// <summary>
		/// 管理しているスプライトのリストを取得
		/// </summary>
		const std::vector<std::unique_ptr<Sprite>>& GetSprites() const { return sprites_; }

	private:
		// SpriteCommonへのポインタ
		SpriteCommon* spriteCommon_ = nullptr;

		// 管理するスプライトのリスト（所有権管理・描画順序用）
		std::vector<std::unique_ptr<Sprite>> sprites_;

		// 名前引き用のマップ（ポインタ参照用）
		std::unordered_map<std::string, Sprite*> namedSprites_;
	};
}