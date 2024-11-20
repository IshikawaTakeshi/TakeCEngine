#include "MyGame.h"
#include "SceneFactory.h"

//====================================================================
//			初期化
//====================================================================

void MyGame::Initialize(const std::wstring& titleName) {

	//FrameWorkの初期化
	TakeCFrameWork::Initialize(titleName);

	sceneFactory_ = std::make_unique<SceneFactory>();
	//シーンマネージャーのセット
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());
	//最初のシーンを設定
	SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
}

//====================================================================
//			終了処理
//====================================================================

void MyGame::Finalize() {
	sceneManager_->Finalize();  //シーンの開放
	TakeCFrameWork::Finalize(); //FrameWorkの終了処理
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