#pragma once

#pragma once
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <type_traits>
#include "BaseUI.h"




namespace TakeC {

	// 前方宣言
	class SpriteManager;

	///=========================================================================
	/// UIマネージャー
	/// BaseUIを継承した複数のUIオブジェクトを一括管理するクラス
	///=========================================================================
	class UIManager {
	public:

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		UIManager() = default;
		~UIManager() = default;

		//=========================================================================
		// functions
		//=========================================================================

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="spriteManager">共有するSpriteManager</param>
		void Initialize(SpriteManager* spriteManager);

		/// <summary>
		/// 全UIの更新
		/// </summary>
		void Update();

		/// <summary>
		/// 全UIの描画
		/// </summary>
		void Draw();

		/// <summary>
		/// ImGuiの一括更新
		/// </summary>
		void UpdateImGui();

		//-------------------------------------------------------------------------
		// 生成・管理メソッド
		//-------------------------------------------------------------------------

		/// <summary>
		/// 新しいUIを作成して登録する（テンプレート版）
		/// 例: uiManager->CreateUI<HPBar>("PlayerHP", ...);
		/// </summary>
		/// <typeparam name="T">BaseUIを継承したクラス</typeparam>
		/// <typeparam name="...Args">Initializeに渡す追加引数の型</typeparam>
		/// <param name="name">登録名 (重複した場合は上書きされます)</param>
		/// <param name="...args">Initializeに渡す追加引数</param>
		/// <returns>作成されたUIへのポインタ</returns>
		template <typename T, typename... Args>
		T* CreateUI(const std::string& name, Args&&... args);


		/// <summary>
		/// 名前を指定してUIを取得する
		/// </summary>
		template <typename T>
		T* GetUI(const std::string& name);


		/// <summary>
		/// 既存のUIインスタンスを登録する（所有権移動）
		/// </summary>
		void RegisterUI(const std::string& name, std::unique_ptr<BaseUI> ui);

		/// <summary>
		/// 全UIのクリア
		/// </summary>
		void Clear();

		/// <summary>
		/// 非アクティブなUI（または削除フラグが立ったUI）を削除する
		/// 必要に応じてUpdateの最後に呼ぶ
		/// </summary>
		//void RemoveDeadUI();

		/// <summary>
		/// 特定のUIのアクティブ状態を設定する
		/// </summary>
		void SetUIActive(const std::string& name, bool isActive);

	private:
		// 共有SpriteManagerへのポインタ
		SpriteManager* spriteManager_ = nullptr;
		// 管理下のUIリスト
		std::map<std::string, std::unique_ptr<BaseUI>> uiMap_;
	};


	//----------------------------------------------------------------------------
	//新しいUIを作成して登録する関数
	//----------------------------------------------------------------------------
	template<typename T, typename ...Args>
	inline T* UIManager::CreateUI(const std::string& name, Args && ...args) {

		static_assert(std::is_base_of<BaseUI, T>::value, "T must inherit from BaseUI");

		// インスタンス生成
		auto newUI = std::make_unique<T>();

		// 初期化 (SpriteManagerポインタ + 任意の引数)
		if (spriteManager_) {
			newUI->Initialize(spriteManager_, std::forward<Args>(args)...);
		}

		T* ptr = newUI.get();
		// mapに登録 (既存があれば上書き)
		uiMap_[name] = std::move(newUI);

		return ptr;
	}

	//----------------------------------------------------------------------------
	//UIを取得する関数
	//----------------------------------------------------------------------------
	template<typename T>
	inline T* UIManager::GetUI(const std::string& name) {
		auto it = uiMap_.find(name);
		if (it != uiMap_.end()) {
			return dynamic_cast<T*>(it->second.get());
		}
		return nullptr;
	}
}