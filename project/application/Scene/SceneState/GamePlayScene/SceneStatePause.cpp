#include "SceneStatePause.h"
#include "application/Scene/GamePlayScene.h"
#include "engine/Base/TakeCFrameWork.h"

using namespace TakeC;

//===================================================================================
// 初期化
//===================================================================================
void SceneStatePause::Initialize([[maybe_unused]] GamePlayScene *scene) {

}

//===================================================================================
// 更新
//===================================================================================
void SceneStatePause::Update([[maybe_unused]] GamePlayScene *scene) {


	if(Input::GetInstance().TriggerButton(0,GamepadButtonType::Start)) {
		RequestTransition(SceneState::PAUSE);
	}
}
