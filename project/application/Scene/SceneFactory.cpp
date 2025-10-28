#include "application/Scene/SceneFactory.h"
#include "application/Scene/TitleScene.h"
#include "application/Scene/GamePlayScene.h"
#include "application/Scene/GameOverScene.h"
#include "application/Scene/GameClearScene.h"
#include "application/Scene/ParticleEditScene.h"
#include "application/Scene/EnemySelectScene.h"

//============================================================================
// シーン生成
//============================================================================
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
	} else if (sceneName == "PARTICLEEDITOR") {
		newScene = std::make_shared<ParticleEditScene>();
	} else if (sceneName == "ENEMYSELECT") {
		newScene = std::make_shared<EnemySelectScene>();
	}
	//初期化
	return newScene;
}
