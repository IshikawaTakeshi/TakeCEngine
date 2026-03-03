#include "GamePlaySceneState.h"
#include "application/Scene/GamePlayScene.h"

//===================================================================================
// BaseScene* → GamePlayScene* 変換して初期化
//===================================================================================
void GamePlaySceneState::Initialize(BaseScene *scene) {
  Initialize(static_cast<GamePlayScene *>(scene));
}

//===================================================================================
// BaseScene* → GamePlayScene* 変換して更新
//===================================================================================
void GamePlaySceneState::Update(BaseScene *scene) {
  Update(static_cast<GamePlayScene *>(scene));
}
