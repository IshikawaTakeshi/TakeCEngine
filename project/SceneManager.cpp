#include "SceneManager.h"
#include <cassert>

SceneManager* SceneManager::instance_ = nullptr;

//========================================================================
//	インスタンス取得
//========================================================================

SceneManager* SceneManager::GetInstance() {
	
	if (instance_ == nullptr) {
		instance_ = new SceneManager();
	}
	return instance_;
}

//========================================================================
//	終了処理
//========================================================================

void SceneManager::Finalize() {
	currentScene_->Finalize();
	currentScene_.reset();
	instance_ = nullptr;
}

//========================================================================
//	更新処理
//========================================================================

void SceneManager::Update() {
	// シーン切り替え
	if (nextScene_) {
		ChangeToNextScene();
	}

	// 実行中のシーンの更新
	if (currentScene_) {
		currentScene_->Update();
	}
}

//================================================================
//	次のシーンに切り替え
//================================================================

void SceneManager::ChangeToNextScene() {
	// 旧シーンの終了
	if (currentScene_) {
		currentScene_->Finalize();
		currentScene_.reset();
	}

	// 次のシーンに切り替え
	currentScene_ = nextScene_;
	nextScene_ = nullptr;

	// シーンマネージャーのセット
	currentScene_->SetSceneManager(this);

	// 次のシーンの初期化
	currentScene_->Initialize();
}

//========================================================================
//	描画処理
//========================================================================

void SceneManager::Draw() {
	currentScene_->Draw();
}

//========================================================================
//	シーンの変更
//========================================================================

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_ != nullptr);
	assert(nextScene_ == nullptr);

	nextScene_ = sceneFactory_->CreateScene(sceneName);
}
