#include "TakeCFrameWork.h"

void TakeCFrameWork::Initialize() {


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
}

void TakeCFrameWork::Finalize() {


	/*==========ImGuiの開放==========*/
#ifdef _DEBUG
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // DEBUG

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
	//Audioの開放
	audio->Finalize();
	//入力の開放
	input->Finalize();
	//SrvManagerの開放
	delete srvManager;
	//directXCommonの開放
	directXCommon->Finalize();
	delete directXCommon;

	//winAppの開放
	winApp->Finalize();
	delete winApp;
}

void TakeCFrameWork::Update() {

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
	
#endif // DEBUG
}

void TakeCFrameWork::Run() {
	//ゲームクラスの生成と初期化
	Initialize();

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		if (IsEndRequest() == true) {
			break;
		}
		Update(); //更新処理
		Draw();   //描画処理
	}

	Finalize();   //終了処理
}
