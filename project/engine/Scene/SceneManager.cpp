#include "SceneManager.h"
#include "base/ImGuiManager.h"
#include "engine/Scene/SceneTransition.h"
#include "engine/2d/SpriteCommon.h"
#include <algorithm>
#include <cassert>
#include <utility>

using namespace TakeC;

//========================================================================
//	インスタンス取得
//========================================================================

SceneManager& SceneManager::GetInstance() {
	
	static SceneManager instance;
	return instance;
}

//========================================================================
//	終了処理
//========================================================================

void SceneManager::Finalize() {
	if (currentScene_) {
		currentScene_->Finalize();
		currentScene_.reset();
	}
	nextScene_.reset();
	sceneFactory_ = nullptr;
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

	SceneTransition::GetInstance()->Update();
}

void SceneManager::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::Begin("SceneManager");

	if (!debugSceneNames_.empty()) {
		itemCurrentIdx_ = std::min(itemCurrentIdx_, debugSceneNames_.size() - 1);
		const std::string& currentItem = debugSceneNames_[itemCurrentIdx_];
		bool changed = false;

		ImGui::Text("selectedScene : %s", currentItem.c_str());
		if (ImGui::BeginCombo("changeScene", currentItem.c_str())) {
			for (size_t index = 0; index < debugSceneNames_.size(); ++index) {
				const bool isSelected = itemCurrentIdx_ == index;
				if (ImGui::Selectable(debugSceneNames_[index].c_str(), isSelected)) {
					itemCurrentIdx_ = index;
					changed = true;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (changed) {
			ChangeScene(debugSceneNames_[itemCurrentIdx_]);
		}
	}

	if (currentScene_) {
		currentScene_->UpdateImGui();
	}
	ImGui::End();
#endif
}

//================================================================
//	次のシーンに切り替え
//================================================================

void SceneManager::ChangeToNextScene() {

	if (SceneTransition::GetInstance()->IsFinished()) {
		// 旧シーンの終了
		if (currentScene_) {
			currentScene_->Finalize();
			currentScene_.reset();
		}

		// 次のシーンに切り替え
		currentScene_ = nextScene_;

		if (currentScene_) {
			// シーンマネージャーのセット
			currentScene_->SetSceneManager(this);

			// 次のシーンの初期化
			currentScene_->Initialize();
		}

		// シーン遷移アニメーションの開始
		SceneTransition::GetInstance()->Start(SceneTransition::TransitionState::FADE_IN, transitionTime_);

		nextScene_ = nullptr;
	}
}

//========================================================================
//	描画処理
//========================================================================

void SceneManager::DrawObject() {
	currentScene_->Draw();
	
}

void SceneManager::DrawBackgroundSprite() {
	TakeC::SpriteCommon::GetInstance().BeginFrame();
	TakeC::SpriteCommon::GetInstance().PreDraw();
	currentScene_->DrawBackgroundSprite();
	TakeC::SpriteCommon::GetInstance().ExecuteDraws();
}

void SceneManager::DrawSprite() {
	TakeC::SpriteCommon::GetInstance().BeginFrame();
	currentScene_->DrawSprite();
	SceneTransition::GetInstance()->Draw();
	TakeC::SpriteCommon::GetInstance().ExecuteDraws();
}

void SceneManager::DrawForegroundSprite() {
	TakeC::SpriteCommon::GetInstance().PreDraw();
	currentScene_->DrawForegroundSprite();
	SceneTransition::GetInstance()->Draw();
	TakeC::SpriteCommon::GetInstance().ExecuteDraws();
}

void SceneManager::DrawShadow() {
	currentScene_->DrawShadow();
}

//========================================================================
//	シーンの変更
//========================================================================

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_ != nullptr);

	// 新しいシーンを作成
	if(nextScene_ == nullptr) {
		SceneTransition::GetInstance()->Start(SceneTransition::TransitionState::FADE_OUT, transitionTime_);
		nextScene_ = sceneFactory_->CreateScene(sceneName);
	}
}

//========================================================================
//	シーンの変更（トランジション時間指定版）
//========================================================================
void SceneManager::ChangeScene(const std::string& sceneName, float transitionTime) {
	assert(sceneFactory_ != nullptr);

	if (nextScene_ == nullptr) {
		SceneTransition::GetInstance()->Start(SceneTransition::TransitionState::FADE_OUT, transitionTime);
		nextScene_ = sceneFactory_->CreateScene(sceneName);
	}
}

void SceneManager::SetDebugSceneNames(std::vector<std::string> sceneNames) {
	debugSceneNames_ = std::move(sceneNames);
	itemCurrentIdx_ = 0;
}
