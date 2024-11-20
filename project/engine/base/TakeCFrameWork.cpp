#include "TakeCFrameWork.h"
#include <cassert>

ParticleManager* TakeCFrameWork::particleManager_ = nullptr;

void TakeCFrameWork::Initialize(const std::wstring& titleName) {

	//タイトルバーの名前の入力
	winApp_ = new WinApp();
	winApp_->Initialize(titleName.c_str());

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

	//ParticleCommon
	particleCommon_ = ParticleCommon::GetInstance();
	particleCommon_->Initialize(directXCommon_, srvManager_);

	//CameraManager
	CameraManager::GetInstance()->Initialize(directXCommon_);

	//ModelManager
	ModelManager::GetInstance()->Initialize(directXCommon_, srvManager_);

	//TextureManager
	TextureManager::GetInstance()->Initialize(directXCommon_, srvManager_);

	//ParticleManager
	particleManager_ = ParticleManager::GetInstance();

#ifdef _DEBUG
	imguiManager_ = new ImGuiManager();
	imguiManager_->Initialize(winApp_, directXCommon_,srvManager_);
#endif
	sceneManager_ = SceneManager::GetInstance();

}

void TakeCFrameWork::Finalize() {
#ifdef _DEBUG
	imguiManager_->Finalize();
#endif

	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	particleManager_->Finalize();

	particleCommon_->Finalize();
	object3dCommon_->Finalize();
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

#ifdef _DEBUG
	imguiManager_->Begin();
#endif

	//入力の更新
	input_->Update();

	//シーンの更新
	sceneManager_->Update();

#ifdef _DEBUG
	imguiManager_->End();
#endif // DEBUG
}

void TakeCFrameWork::Run(const std::wstring& titleName) {
	//ゲームクラスの生成と初期化
	Initialize(titleName);

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

ParticleManager* TakeCFrameWork::GetParticleManager() {
	assert(particleManager_ != nullptr);
	return particleManager_;
}
