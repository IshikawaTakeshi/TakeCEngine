#pragma once
#include <memory>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

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

	void ChangeToNextScene();

	//========================================================================
	//	アクセッサ
	//========================================================================

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
};