#include "TakeCFrameWork.h"
#include <cassert>

std::unique_ptr<ParticleManager> TakeCFrameWork::particleManager_ = nullptr;
std::unique_ptr<Animator> TakeCFrameWork::animator_ = nullptr;

void TakeCFrameWork::Initialize(const std::wstring& titleName) {

	//タイトルバーの名前の入力
	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize(titleName.c_str());

	////DirectX初期化
	directXCommon_ = std::make_unique<DirectXCommon>();
	directXCommon_->Initialize(winApp_.get());

	//SrvManager
	srvManager_ = std::make_unique<SrvManager>();
	srvManager_->Initialize(directXCommon_.get());

	//入力初期化
	input_ = Input::GetInstance();
	input_->Initialize(winApp_.get());

	//Audio
	audio_ = AudioManager::GetInstance();
	audio_->Initialize();

	//SpriteCommon
	spriteCommon_ = SpriteCommon::GetInstance();
	spriteCommon_->Initialize(directXCommon_.get());

	//Object3dCommon
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->Initialize(directXCommon_.get());

	//ParticleCommon
	particleCommon_ = ParticleCommon::GetInstance();
	particleCommon_->Initialize(directXCommon_.get(), srvManager_.get());

	animator_ = std::make_unique<Animator>();

	//CameraManager
	CameraManager::GetInstance()->Initialize(directXCommon_.get());

	//ModelManager
	ModelManager::GetInstance()->Initialize(directXCommon_.get(), srvManager_.get());

	//TextureManager
	TextureManager::GetInstance()->Initialize(directXCommon_.get(), srvManager_.get());

	//ParticleManager
	particleManager_ = std::make_unique<ParticleManager>();


#ifdef _DEBUG
	imguiManager_ = new ImGuiManager();
	imguiManager_->Initialize(winApp_.get(), directXCommon_.get(), srvManager_.get());
#endif
	sceneManager_ = SceneManager::GetInstance();

}

void TakeCFrameWork::Finalize() {
#ifdef _DEBUG
	imguiManager_->Finalize();
#endif

	animator_->Finalize();
	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	particleManager_->Finalize();

	particleCommon_->Finalize();
	object3dCommon_->Finalize();
	spriteCommon_->Finalize();
	sceneFactory_.reset();
	//Audioの開放
	audio_->Finalize();
	//入力の開放
	input_->Finalize();
	//SrvManagerの開放
	srvManager_.reset();
	//directXCommonの開放
	directXCommon_->Finalize();
	directXCommon_.reset();

	//winAppの開放
	winApp_->Finalize();
	winApp_.reset();
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
	return particleManager_.get();
}

Animator* TakeCFrameWork::GetAnimator() {
	assert(animator_ != nullptr);
	return animator_.get();
}
