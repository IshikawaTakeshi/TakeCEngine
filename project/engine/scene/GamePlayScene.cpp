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
	camera0_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera0", *camera0_);

	//Camera1
	camera1_ = std::make_shared<Camera>();
	camera1_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera1_->SetTranslate({ 5.0f,0.0f,-1.0f });
	camera1_->SetRotate({ 0.0f,-1.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera1", *camera1_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//Model読み込み
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("skydome.obj");

	//Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/uvChecker.png");

	//Object3d
	object3d = std::make_shared<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance(), "skydome.obj");
	object3d->SetScale({ 0.002f,0.002f,0.002f });

	object3d1 = std::make_shared<Object3d>();
	object3d1->Initialize(Object3dCommon::GetInstance(), "axis.obj");

	//Particle3d
	particle3d_ = std::make_unique<Particle3d>();
	particle3d_->Initialize(ParticleCommon::GetInstance(), "plane.obj");
	particle3d_->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	sprite_.reset();    //スプライトの解放

	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	AudioManager::GetInstance()->SoundUnload(&soundData1); //音声データ解放
}

//====================================================================
//			更新処理
//====================================================================

void GamePlayScene::Update() {
	//ImGuiの更新
#ifdef _DEBUG
	CameraManager::GetInstance()->UpdateImGui();
	//sprite_->UpdateImGui(0);
	object3d->UpdateImGui(0);
	object3d1->UpdateImGui(1);
	particle3d_->UpdateImGui();

#endif // DEBUG

	//オーディオ再生
	if (Input::GetInstance()->TriggerKey(DIK_A)) {
		AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), soundData1);
	}

	//カメラの更新
	CameraManager::GetInstance()->Update();

	//sprite_->Update(); 	//Spriteの更新
	object3d->Update(); //3Dオブジェクトの更新
	object3d1->Update();
	particle3d_->Update(); //パーティクルの更新

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();     //Spriteの描画前処理
	//sprite_->Draw();              //スプライトの描画

	Object3dCommon::GetInstance()->PreDraw();   //Object3dの描画前処理
	object3d->Draw();             //3Dオブジェクトの描画
	object3d1->Draw();

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理

	particle3d_->Draw();          //パーティクルの描画
}