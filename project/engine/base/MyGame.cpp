#include "MyGame.h"

//====================================================================
//			初期化
//====================================================================

void MyGame::Initialize() {

	TakeCFrameWork::Initialize();

	//サウンドデータ
	soundData1 = audio->SoundLoadWave("Resources/audioSources/fanfare.wav");

	//Camera0
	camera0 = std::make_shared<Camera>();
	camera0->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0->SetRotate({ 0.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera(*camera0);

	//Camera1
	camera1 = std::make_shared<Camera>();
	camera1->SetTranslate({ 5.0f,0.0f,-10.0f });
	camera1->SetRotate({ 0.0f,-0.4f,0.0f });
	CameraManager::GetInstance()->AddCamera(*camera1);

	//デフォルトカメラの設定
	object3dCommon->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//Model読み込み
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");

	//Sprite
	sprite = std::make_shared<Sprite>();
	sprite->Initialize(spriteCommon, "Resources/uvChecker.png");

	//3dObject
	object3d = std::make_shared<Object3d>();
	object3d->Initialize(object3dCommon, "axis.obj");
	object3d->SetScale({ 0.5f,0.5f,0.5f });

	object3d1 = std::make_shared<Object3d>();
	object3d1->Initialize(object3dCommon, "plane.obj");
	object3d1->SetScale({ 0.5f,0.5f,0.5f });
}

//====================================================================
//			終了処理
//====================================================================

void MyGame::Finalize() {
	sprite.reset();
	object3d.reset();
	object3d1.reset();
	audio->SoundUnload(&soundData1); //音声データ解放
	TakeCFrameWork::Finalize();
}

//====================================================================
//			更新処理
//====================================================================

void MyGame::Update() {

	TakeCFrameWork::Update();

	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	sprite->UpdateImGui(0);
	object3d->UpdateImGui(0);
	object3d1->UpdateImGui(1);


	//オーディオ再生
	if (input->TriggerKey(DIK_A)) {
		audio->SoundPlayWave(audio->GetXAudio2(), soundData1);
	}
	//入力更新
	input->Update();

	//カメラの更新
	CameraManager::GetInstance()->Update();

	//カメラの切り替え
	if (input->GetInstance()->TriggerKey(DIK_1)) {
		CameraManager::GetInstance()->SetActiveCamera(0);
	} else if (input->GetInstance()->TriggerKey(DIK_2)) {
		CameraManager::GetInstance()->SetActiveCamera(1);
	}

	//Spriteの更新
	sprite->Update();

	//3dObjectの更新
	object3d->Update();
	object3d1->Update();

#ifdef _DEBUG

#endif // DEBUG
}

//====================================================================
//			描画処理
//====================================================================

void MyGame::Draw() {

	//========================== 描画処理　==========================//

	 //描画前処理
	directXCommon->PreDraw();
	srvManager->PreDraw();       //SRV描画前処理
	spriteCommon->PreDraw();     //Spriteの描画前処理
	object3dCommon->PreDraw();   //Object3dの描画前処理

	//Spriteの描画
	sprite->Draw();

	object3d->Draw();            //3Dオブジェクトの描画
	object3d1->Draw();           //3Dオブジェクトの描画

	//描画後処理
	directXCommon->PostDraw();
}
