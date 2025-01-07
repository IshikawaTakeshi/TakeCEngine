#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"

std::shared_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName) {

    //次のシーンの生成
	std::shared_ptr<BaseScene> newScene = nullptr;
	if (sceneName == "TITLE") {
		newScene = std::make_shared<TitleScene>();
	} else if (sceneName == "GAMEPLAY") {
		newScene = std::make_shared<GamePlayScene>();
	} else if (sceneName == "GAMEOVER") {
		newScene = std::make_shared<GameOverScene>();
	} else if (sceneName == "GAMECLEAR") {
		newScene = std::make_shared<GameClearScene>();
	}
	return newScene;
}
