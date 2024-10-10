#include "MyGame.h"

//====================================================================
//			初期化
//====================================================================

void MyGame::Initialize() {

	//FrameWorkの初期化
	TakeCFrameWork::Initialize();

	currentScene_ = std::make_shared<TitleScene>();
	sceneManager_->SetNextScene(currentScene_);
}

//====================================================================
//			終了処理
//====================================================================

void MyGame::Finalize() {

	TakeCFrameWork::Finalize();      //FrameWorkの終了処理
}

//====================================================================
//			更新処理
//====================================================================

void MyGame::Update() {

	//FrameWorkの更新
	TakeCFrameWork::Update();
}

//====================================================================
//			描画処理
//====================================================================

void MyGame::Draw() {

	 //描画前処理
	directXCommon_->PreDraw();
	srvManager_->PreDraw();       //SRV描画前処理
	
	sceneManager_->Draw();

	//描画後処理
	directXCommon_->PostDraw();
}