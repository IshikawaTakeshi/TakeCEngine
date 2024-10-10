#include "GamePlayScene.h"
#include "TitleScene.h"
#include "SceneManager.h"

//====================================================================
//			初期化
//====================================================================

void GamePlayScene::Initialize() {
	//サウンドデータ
	soundData1 = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/fanfare.wav");

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera0", *camera0_);

	//Camera1
	camera1_ = std::make_shared<Camera>();
	camera1_->SetTranslate({ 5.0f,0.0f,-10.0f });
	camera1_->SetRotate({ 0.0f,-0.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera1", *camera1_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//Model読み込み
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");

	//Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/uvChecker.png");

	//3dObject
	object3d = std::make_shared<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance(), "axis.obj");
	object3d->SetScale({ 0.5f,0.5f,0.5f });

	object3d1 = std::make_shared<Object3d>();
	object3d1->Initialize(Object3dCommon::GetInstance(), "plane.obj");
	object3d1->SetScale({ 0.5f,0.5f,0.5f });
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	sprite_.reset();    //スプライトの解放
	object3d.reset();  //3Dオブジェクトの解放
	object3d1.reset();

	AudioManager::GetInstance()->SoundUnload(&soundData1); //音声データ解放

}

void GamePlayScene::Update() {
	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	sprite_->UpdateImGui(0);
	object3d->UpdateImGui(0);
	object3d1->UpdateImGui(1);

	//オーディオ再生
	if (Input::GetInstance()->TriggerKey(DIK_A)) {
		AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), soundData1);
	}

	//カメラの更新
	CameraManager::GetInstance()->Update();

	sprite_->Update(); 	//Spriteの更新
	object3d->Update(); //3dObjectの更新
	object3d1->Update();

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//ゲームプレイシーンの生成
		std::shared_ptr<BaseScene> scece = std::make_shared<TitleScene>();
		//シーン切り替え依頼
		SceneManager::GetInstance()->SetNextScene(scece);
	}
}

void GamePlayScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();     //Spriteの描画前処理
	Object3dCommon::GetInstance()->PreDraw();   //Object3dの描画前処理


	sprite_->Draw();              //スプライトの描画
	object3d->Draw();            //3Dオブジェクトの描画
	object3d1->Draw();           //3Dオブジェクトの描画

}
