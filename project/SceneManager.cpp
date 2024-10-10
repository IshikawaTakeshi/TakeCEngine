#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
	
	if (instance_ == nullptr) {
		instance_ = new SceneManager();
	}
	return instance_;
}

void SceneManager::Finalize() {
	currentScene_->Finalize();
	currentScene_.reset();
}

void SceneManager::Update() {

	//シーン切り替え
	if (nextScene_) {
		// 旧シーンの終了
		if (currentScene_) {
			currentScene_->Finalize();
			currentScene_.reset();
		}

		currentScene_ = nextScene_;
		nextScene_ = nullptr;
		//シーンマネージャーのセット
		currentScene_->SetSceneManager(this);
		//次のシーンの初期化
		currentScene_->Initialize();
	}

	//実行中のシーンの更新
	currentScene_->Update();
}

void SceneManager::Draw() {
	currentScene_->Draw();
}