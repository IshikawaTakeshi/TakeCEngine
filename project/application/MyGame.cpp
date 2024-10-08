#include "MyGame.h"

//====================================================================
//			初期化
//====================================================================

void MyGame::Initialize() {

	//タイトルバーの名前の入力
	winApp = new WinApp();
	winApp->Initialize(L"CG2_08_01");

	////DirectX初期化
	directXCommon = new DirectXCommon();
	directXCommon->Initialize(winApp);

	//SrvManager
	srvManager = new SrvManager();
	srvManager->Initialize(directXCommon);

	//入力初期化
	input = Input::GetInstance();
	input->Initialize(winApp);

	//Audio
	audio = std::make_shared<Audio>();
	audio->Initialize();

	//SpriteCommon
	spriteCommon = SpriteCommon::GetInstance();
	spriteCommon->Initialize(directXCommon);

	//Object3dCommon
	object3dCommon = Object3dCommon::GetInstance();
	object3dCommon->Initialize(directXCommon);

	//ModelManager
	ModelManager::GetInstance()->Initialize(directXCommon, srvManager);

	//TextureManager
	TextureManager::GetInstance()->Initialize(directXCommon, srvManager);



#pragma region ImGui初期化
#ifdef _DEBUG
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(directXCommon->GetDevice(),
		directXCommon->GetBufferCount(),
		directXCommon->GetRtvFormat(),
		srvManager->GetSrvHeap(),
		srvManager->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart(),
		srvManager->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart()
	);
#endif // DEBUG
#pragma endregion

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


	/*==========ImGuiの開放==========*/
#ifdef _DEBUG
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // DEBUG

	audio->Finalize(soundData1);

	delete srvManager;
	sprite.reset();
	object3d.reset();
	object3d1.reset();


	//テクスチャマネージャの開放
	TextureManager::GetInstance()->Finalize();
	//ModelManagerの開放
	ModelManager::GetInstance()->Finalize();
	//CameraManagerの開放
	CameraManager::GetInstance()->Finalize();


	//Object3dCommonの開放
	object3dCommon->Finalize();
	//SpriteCommonの開放aa
	spriteCommon->Finalize();
	//入力の開放
	input->Finalize();
	//directXCommonの開放
	directXCommon->Finalize();
	delete directXCommon;

	//winAppの開放
	winApp->Finalize();
	delete winApp;

}

//====================================================================
//			更新処理
//====================================================================

void MyGame::Update() {

	//メッセージ処理
	if (winApp->ProcessMessage()) {
		//ウィンドウの×ボタンが押されたらループを抜ける
		isEnd_ = true;
	}

	//========================== 更新処理　==========================//

#ifdef _DEBUG
		//ImGui受付開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム特有の処理に置き換える
		//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	sprite->UpdateImGui(0);
	object3d->UpdateImGui(0);
	object3d1->UpdateImGui(1);
#endif // DEBUG

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
