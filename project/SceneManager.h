#pragma once
#include "BaseScene.h"
#include <memory>

////////////////////
///	シーン管理クラス
////////////////////
class SceneManager {
public:

	static SceneManager* GetInstance();

	void Finalize();

	void Update();

	void Draw();

	void SetNextScene(std::shared_ptr<BaseScene> nextScene) { nextScene_ = nextScene; }

private:

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

private:

	static SceneManager* instance_;
	std::shared_ptr<BaseScene> currentScene_ = nullptr;
	std::shared_ptr<BaseScene> nextScene_ = nullptr;
};

