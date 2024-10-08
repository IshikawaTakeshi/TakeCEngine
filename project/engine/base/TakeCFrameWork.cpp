#include "TakeCFrameWork.h"

void TakeCFrameWork::Initialize() {


	//タイトルバーの名前の入力
	winApp_ = new WinApp();
	winApp_->Initialize(L"CG2_08_01");

	////DirectX初期化
	directXCommon_ = new DirectXCommon();
	directXCommon_->Initialize(winApp_);

	//SrvManager
	srvManager_ = new SrvManager();
	srvManager_->Initialize(directXCommon_);

	//入力初期化
	input_ = Input::GetInstance();
	input_->Initialize(winApp_);

	//Audio
	audio_ = AudioManager::GetInstance();
	audio_->Initialize();

	//SpriteCommon
	spriteCommon_ = SpriteCommon::GetInstance();
	spriteCommon_->Initialize(directXCommon_);

	//Object3dCommon
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->Initialize(directXCommon_);

	//ModelManager
	ModelManager::GetInstance()->Initialize(directXCommon_, srvManager_);

	//TextureManager
	TextureManager::GetInstance()->Initialize(directXCommon_, srvManager_);



#pragma region ImGui初期化
#ifdef _DEBUG
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	ImGui_ImplDX12_Init(directXCommon_->GetDevice(),
		directXCommon_->GetBufferCount(),
		directXCommon_->GetRtvFormat(),
		srvManager_->GetSrvHeap(),
		srvManager_->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart(),
		srvManager_->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart()
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
	object3dCommon_->Finalize();
	//SpriteCommonの開放aa
	spriteCommon_->Finalize();
	//Audioの開放
	audio_->Finalize();
	//入力の開放
	input_->Finalize();
	//SrvManagerの開放
	delete srvManager_;
	//directXCommonの開放
	directXCommon_->Finalize();
	delete directXCommon_;

	//winAppの開放
	winApp_->Finalize();
	delete winApp_;
}

void TakeCFrameWork::Update() {

	//メッセージ処理
	if (winApp_->ProcessMessage()) {
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
