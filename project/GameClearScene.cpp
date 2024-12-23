#include "GameClearScene.h"
#include "scene/SceneManager.h"
#include "ImGuiManager.h"
#include "Input.h"

void GameClearScene::Initialize() {

}

void GameClearScene::Finalize() {
}

void GameClearScene::Update() {
	ImGui::Begin("GameClearScene");
	ImGui::Text("Game Clear!!");
	ImGui::Text("push Enter : Title");
	ImGui::End();

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
}

void GameClearScene::Draw() {

}
