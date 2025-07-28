#pragma once
#include <memory>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "scene//LevelData.h"
#include "engine/Entity/LevelObject/Levelobject.h"

////////////////////
///	シーン管理クラス
////////////////////
class SceneManager {
public:

	static SceneManager* GetInstance();

	void Finalize();

	void Update();

	void UpdateImGui();

	void Draw();

	void ChangeScene(const std::string& sceneName);
	void ChangeScene(const std::string& sceneName, float transitionTime);

	void ChangeToNextScene();

	void LoadLevelData(const std::string& sceneName);

	//========================================================================
	//	アクセッサ
	//========================================================================

	std::map<std::string,std::unique_ptr<LevelObject>>& GetLevelObjects() { return levelObjects_; }

	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }

private:

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

private:

	static SceneManager* instance_;
	std::shared_ptr<BaseScene> currentScene_ = nullptr;
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