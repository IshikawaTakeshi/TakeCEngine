#pragma once
#include <memory>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "scene//LevelData.h"
#include "engine/Entity/LevelObject/Levelobject.h"

//========================================================================
//	SceneManager class
//========================================================================
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
	void DrawSprite();

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

	/// <summary>
	/// レベルデータの読み込み
	/// </summary>
	/// <param name="sceneName"></param>
	void LoadLevelData(const std::string& sceneName);

	//========================================================================
	//	accessors
	//========================================================================

	//----- getter --------------------

	/// 現在のシーンの取得
	std::map<std::string,std::unique_ptr<LevelObject>>& GetLevelObjects() { return levelObjects_; }

	//----- setter --------------------

	/// シーンファクトリーのセット
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }


private:

	//現在のシーン
	std::shared_ptr<BaseScene> currentScene_ = nullptr;
	//次のシーン
	std::shared_ptr<BaseScene> nextScene_ = nullptr;
	//シーンファクトリー(借りてくる)
	AbstractSceneFactory* sceneFactory_ = nullptr;

	//ImGuiCombo用インデックス
	uint32_t itemCurrentIdx = 0;
	// 遷移時間
	float transitionTime_ = 0.5f;
	// レベルデータの格納
	LevelData* levelData_; 
	// レベル内のオブジェクトのリスト
	std::map<std::string,std::unique_ptr<LevelObject>> levelObjects_;
};