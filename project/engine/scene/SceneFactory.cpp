#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"

std::shared_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName) {

    //次のシーンの生成
	std::shared_ptr<BaseScene> newScene = nullptr;
	if (sceneName == "TITLE") {
		newScene = std::make_shared<TitleScene>();
	} else if (sceneName == "GAMEPLAY") {
		newScene = std::make_shared<GamePlayScene>();
	} 
	return newScene;
}
