#pragma once
#include <memory>
#include <string>
#include <vector>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

//========================================================================
//	SceneManager class
//========================================================================
/// <summary>
/// Sceneに関する生成、更新、破棄を一元管理するクラスです。
/// </summary>
class SceneManager {
private:

	//コピーコンストラクタ・代入演算子禁止
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

public:

	//=====================================================================
	//	functions
	//=====================================================================

	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static SceneManager& GetInstance();

	/// <summary>
	/// インスタンスの解放
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiの更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理
	/// </summary>
	void DrawObject();

	/// <summary>
	/// スプライト描画処理
	/// </summary>
	void DrawBackgroundSprite();
	void DrawSprite();
	void DrawForegroundSprite();

	void DrawShadow();

	/// <summary>
	/// シーンの変更
	/// </summary>
	/// <param name="sceneName"></param>
	void ChangeScene(const std::string& sceneName);

	/// <summary>
	/// シーンの変更（トランジション時間指定版）
	/// </summary>
	/// <param name="sceneName"></param>
	/// <param name="transitionTime"></param>
	void ChangeScene(const std::string& sceneName, float transitionTime);

	/// <summary>
	/// 次のシーンへ変更
	/// </summary>
	void ChangeToNextScene();

	/// シーンファクトリーのセット
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }

	/// デバッグUIに表示するシーン名を登録
	void SetDebugSceneNames(std::vector<std::string> sceneNames);


private:

	//現在のシーン
	std::shared_ptr<BaseScene> currentScene_ = nullptr;
	//次のシーン
	std::shared_ptr<BaseScene> nextScene_ = nullptr;
	//シーンファクトリー(借りてくる)
	AbstractSceneFactory* sceneFactory_ = nullptr;

	//ImGuiCombo用インデックス
	size_t itemCurrentIdx_ = 0;
	std::vector<std::string> debugSceneNames_;
	// 遷移時間
	float transitionTime_ = 0.5f;
};
