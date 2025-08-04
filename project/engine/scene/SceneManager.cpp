#include "SceneManager.h"
#include "base/ImGuiManager.h"
#include "base//ModelManager.h"
#include "base/TakeCFrameWork.h"
#include "3d/Object3dCommon.h"
#include "scene/SceneTransition.h"
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

	SceneTransition::GetInstance()->Update();
}

void SceneManager::UpdateImGui() {
#ifdef _DEBUG

	//コンボボックスの項目
	std::vector<std::string> items = { "GAMEPLAY","TITLE", "GAMEOVER","GAMECLEAR","PARTICLEEDITOR"};
	//現在の項目
	std::string& currentItem = items[itemCurrentIdx];
	//変更があったかどうか
	bool changed = false;

	//コンボボックスの表示
	ImGui::Begin("SceneManager");
	ImGui::Text("currentScene : %s", currentItem.c_str());
	if (ImGui::BeginCombo("changeScene", currentItem.c_str())) {
		for (int n = 0; n < items.size(); n++) {
			const bool is_selected = (currentItem == items[n]);
			if (ImGui::Selectable(items[n].c_str(), is_selected)) {
				currentItem = items[itemCurrentIdx];
				itemCurrentIdx = n;
				changed = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		currentItem = items[itemCurrentIdx];
		ImGui::EndCombo();
	}
	currentScene_->UpdateImGui();
	ImGui::End();

	//変更があった場合
	if (changed) {
		switch (itemCurrentIdx) {
		case 0:
			nextScene_ = sceneFactory_->CreateScene("GAMEPLAY");
			break;
		case 1:
			nextScene_ = sceneFactory_->CreateScene("TITLE");
			break;
		case 2:
			nextScene_ = sceneFactory_->CreateScene("GAMEOVER");
			break;
		case 3:
			nextScene_ = sceneFactory_->CreateScene("GAMECLEAR");
			break;
		case 4:
			nextScene_ = sceneFactory_->CreateScene("PARTICLEEDITOR");
			break;
		}
	}
#endif // _DEBUG
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

		// シーンマネージャーのセット
		currentScene_->SetSceneManager(this);

		// 次のシーンの初期化
		currentScene_->Initialize();

		// シーン遷移アニメーションの開始
		SceneTransition::GetInstance()->Start(SceneTransition::TransitionState::FADE_IN, transitionTime_);

		nextScene_ = nullptr;
	}
}

void SceneManager::LoadLevelData(const std::string& sceneName) {

	levelData_ = TakeCFrameWork::GetJsonLoader()->LoadLevelFile(sceneName);

	for (auto& objectData : levelData_->objects) {
		Model* model = nullptr;
		model = ModelManager::GetInstance()->FindModel(objectData.file_name);
		std::pair<std::string, std::unique_ptr<LevelObject>> newObject = { objectData.name, std::make_unique<LevelObject>() };
		newObject.second->Initialize(Object3dCommon::GetInstance(), objectData.file_name);
		newObject.second->SetName(objectData.name);
		if (objectData.collider.isValid) {

			// コライダーの種類によって初期化
			std::visit([&](auto&& colliderData) {
				// 型に応じてコライダーを初期化
				using T = std::decay_t<decltype(colliderData)>;

				// コライダーの型に応じて処理を分岐,廃棄する
				if constexpr (std::is_same_v<T, LevelData::BoxCollider>) {
					newObject.second->CollisionInitialize(colliderData);
				} else if constexpr (std::is_same_v<T, LevelData::SphereCollider>) {
					newObject.second->CollisionInitialize(colliderData);
				}
				}, objectData.collider.colliderData);
		}
		newObject.second->SetTranslate(objectData.translation);
		newObject.second->SetRotate(objectData.rotation);
		newObject.second->SetScale(objectData.scale);
		newObject.second->GetObject3d()->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });
		newObject.second->GetObject3d()->GetModel()->GetMesh()->GetMaterial()->SetEnvCoefficient(0.0f);

		levelObjects_.insert(std::move(newObject));
	}
}

//========================================================================
//	描画処理
//========================================================================

void SceneManager::Draw() {
	currentScene_->Draw();
	SceneTransition::GetInstance()->Draw();
}

//========================================================================
//	シーンの変更
//========================================================================

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_ != nullptr);

	// 既存のオブジェクトをクリア
	levelObjects_.clear();

	// 新しいシーンを作成
	if(nextScene_ == nullptr) {
		SceneTransition::GetInstance()->Start(SceneTransition::TransitionState::FADE_OUT, transitionTime_);
		nextScene_ = sceneFactory_->CreateScene(sceneName);
	}
}

void SceneManager::ChangeScene(const std::string& sceneName, float transitionTime) {
	assert(sceneFactory_ != nullptr);

	// 既存のオブジェクトをクリア
	levelObjects_.clear();

	if (nextScene_ == nullptr) {
		SceneTransition::GetInstance()->Start(SceneTransition::TransitionState::FADE_OUT, transitionTime);
		nextScene_ = sceneFactory_->CreateScene(sceneName);
	}
}