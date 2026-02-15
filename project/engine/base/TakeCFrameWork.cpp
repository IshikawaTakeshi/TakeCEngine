#include "TakeCFrameWork.h"
#include <cassert>

using namespace TakeC;

//Clockの宣言
using Clock = std::chrono::high_resolution_clock;

std::unique_ptr<TakeC::AnimationManager> TakeCFrameWork::animationManager_ = nullptr;
std::unique_ptr<TakeC::JsonLoader> TakeCFrameWork::jsonLoader_ = nullptr;
std::unique_ptr<TakeC::LightManager> TakeCFrameWork::lightManager_ = nullptr;
std::unique_ptr<TakeC::ParticleManager> TakeCFrameWork::particleManager_ = nullptr;
std::unique_ptr<TakeC::PrimitiveDrawer> TakeCFrameWork::primitiveDrawer_ = nullptr;
std::unique_ptr<TakeC::PostEffectManager> TakeCFrameWork::postEffectManager_= nullptr;
std::unique_ptr<TakeC::WireFrame> TakeCFrameWork::wireFrame_ = nullptr;
std::unique_ptr<TakeC::SpriteManager> TakeCFrameWork::spriteManager_ = nullptr;
std::unique_ptr<TakeC::UIManager> TakeCFrameWork::uiManager_ = nullptr;
std::unique_ptr<TakeC::EventManager> TakeCFrameWork::eventManager_ = nullptr;

std::chrono::steady_clock::time_point TakeCFrameWork::gameTime_ = Clock::now();
float TakeCFrameWork::kDeltaTime = 0.016f; // 60FPSを基準にしたデルタタイム

//====================================================================
//			初期化
//====================================================================

void TakeCFrameWork::Initialize(const std::wstring& titleName) {

	//タイトルバーの名前の入力
	winApp_ = std::make_unique<TakeC::WinApp>();
	winApp_->Initialize(titleName.c_str());

	////DirectX初期化
	directXCommon_ = std::make_unique<TakeC::DirectXCommon>();
	directXCommon_->Initialize(winApp_.get());
	//SrvManager
	srvManager_ = std::make_unique<TakeC::SrvManager>();
	srvManager_->Initialize(directXCommon_.get());

	//ResourceBarrier
	ResourceBarrier::GetInstance().Initialize(directXCommon_.get());

	
	//入力初期化
	input_ = &Input::GetInstance();
	input_->Initialize(winApp_.get());

	//Audio
	audio_ = &AudioManager::GetInstance();
	audio_->Initialize();

	//JsonLoader
	jsonLoader_ = std::make_unique<JsonLoader>();

	//lightManager
	lightManager_ = std::make_unique<TakeC::LightManager>();
	lightManager_->Initialize(directXCommon_.get(), srvManager_.get());

	//SpriteCommon
	spriteCommon_ = &SpriteCommon::GetInstance();
	spriteCommon_->Initialize(directXCommon_.get());

	//Object3dCommon
	object3dCommon_ = &Object3dCommon::GetInstance();
	object3dCommon_->Initialize(directXCommon_.get(),lightManager_.get());

	//ParticleCommon
	particleCommon_ = &ParticleCommon::GetInstance();
	particleCommon_->Initialize(directXCommon_.get(), srvManager_.get(),lightManager_.get());

	//AnimationManager
	animationManager_ = std::make_unique<AnimationManager>();

	//CameraManager
	TakeC::CameraManager::GetInstance().Initialize(directXCommon_.get());

	//ModelManager
	TakeC::ModelManager::GetInstance().Initialize(directXCommon_.get(), srvManager_.get());

	//TextureManager
	TakeC::TextureManager::GetInstance().Initialize(directXCommon_.get(), srvManager_.get());

	//SpriteManager
	spriteManager_ = std::make_unique<SpriteManager>();
	spriteManager_->Initialize(spriteCommon_);

	//UIManager
	uiManager_ = std::make_unique<UIManager>();
	uiManager_->Initialize(spriteManager_.get());	

	//EventManager
	eventManager_ = std::make_unique<EventManager>();

	//PrimitiveDrawer
	primitiveDrawer_ = std::make_unique<PrimitiveDrawer>();
	primitiveDrawer_->Initialize(directXCommon_.get(), srvManager_.get());

	//ParticleManager
	particleManager_ = std::make_unique<ParticleManager>();
	particleManager_->Initialize(particleCommon_,primitiveDrawer_.get());

	//RenderTexture
	renderTexture_ = std::make_unique<RenderTexture>();
	renderTexture_->Initialize(directXCommon_.get(), srvManager_.get());
	//PostEffectFactory
	postEffectFactory_ = std::make_unique<PostEffectFactory>();
	//PostEffectManager
	postEffectManager_ = std::make_unique<PostEffectManager>();
	postEffectManager_->Initialize(directXCommon_.get(), srvManager_.get(),renderTexture_.get());
	postEffectManager_->SetPostEffectFactory(postEffectFactory_.get());

	//WireFrame
	wireFrame_ = std::make_unique<WireFrame>();
	wireFrame_->Initialize(directXCommon_.get());


#ifdef _DEBUG
	imguiManager_ = new ImGuiManager();
	imguiManager_->Initialize(winApp_.get(), directXCommon_.get(), srvManager_.get());
#endif

	//sceneManager
	sceneManager_ = &SceneManager::GetInstance();

	//sceneTransition
	sceneTransition_ = SceneTransition::GetInstance();
	sceneTransition_->Initialize();

}

//====================================================================
//			終了処理
//====================================================================

void TakeCFrameWork::Finalize() {
#ifdef _DEBUG
	imguiManager_->Finalize();
#endif

	wireFrame_->Finalize();
	animationManager_->Finalize();
	TakeC::TextureManager::GetInstance().Finalize();
	TakeC::ModelManager::GetInstance().Finalize();
	CameraManager::GetInstance().Finalize();
	postEffectManager_->Finalize();
	renderTexture_.reset();
	particleManager_->Finalize();
	primitiveDrawer_->Finalize();
	particleCommon_->Finalize();
	object3dCommon_->Finalize();
	lightManager_->Finalize();
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

//====================================================================
//			更新処理
//====================================================================

void TakeCFrameWork::Update() {

	//メッセージ処理
	if (winApp_->ProcessMessage()) {
		//ウィンドウの×ボタンが押されたらループを抜ける
		isEnd_ = true;
	}

	kDeltaTime = 0.016f * timeScale_;

#ifdef _DEBUG
	imguiManager_->Begin();

	ImGui::Begin("FrameWork");
	directXCommon_->DrawFPS();
	ImGui::Text("DeltaTime: %.4f", kDeltaTime);
	ImGui::DragFloat("TimeScale", &timeScale_, 0.01f, 0.0f, 5.0f);
	ImGui::End();
#endif
	
	//入力の更新
	input_->Update();

	//シーンの更新
	sceneManager_->Update();

#ifdef _DEBUG
	sceneManager_->UpdateImGui();
	postEffectManager_->UpdateImGui();

	//ImGuiのRenderTextureのSRVインデックスを設定
	imguiManager_->SetRenderTextureIndex(postEffectManager_->GetFinalOutputSrvIndex()); 

	imguiManager_->DrawDebugScreen();
	imguiManager_->End();
#endif // DEBUG
}

//====================================================================
//			実行処理
//====================================================================

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

		if(Input::GetInstance().TriggerKey(DIK_ESCAPE)) {
			break;
		}
	}

	Finalize();   //終了処理
}

//====================================================================
//			ゲーム起動時間の取得
//====================================================================
float TakeCFrameWork::GetGameTime() {
	
	//現在の時間を取得
	auto now = Clock::now();

	//経過時間をミリ秒単位で計算
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - gameTime_).count();

	//秒に変換
	return duration / 1000.0f;
}

//====================================================================
//			ParticleManagerの取得
//====================================================================

TakeC::ParticleManager* TakeCFrameWork::GetParticleManager() {
	assert(particleManager_ && "ParticleManagerが生成されていません");
	return particleManager_.get();
}

//====================================================================
//			AnimationManagerの取得
//====================================================================

TakeC::AnimationManager* TakeCFrameWork::GetAnimationManager() {
	assert(animationManager_ && "AnimationManagerが生成されていません");
	return animationManager_.get();
}

//====================================================================
//			JsonLoaderの取得
//====================================================================

TakeC::JsonLoader* TakeCFrameWork::GetJsonLoader() {
	assert(jsonLoader_ && "JsonLoaderが生成されていません");
	return jsonLoader_.get();
}

//====================================================================
//			PrimitiveDrawerの取得
//====================================================================

TakeC::PrimitiveDrawer* TakeCFrameWork::GetPrimitiveDrawer() {
	assert(primitiveDrawer_ && "PrimitiveDrawerが生成されていません");
	return primitiveDrawer_.get();
}

//====================================================================
//			PostEffectManagerの取得
//====================================================================

TakeC::PostEffectManager* TakeCFrameWork::GetPostEffectManager() {
	assert(postEffectManager_ && "PostEffectManagerが生成されていません");
	return postEffectManager_.get();
}

//====================================================================
//			WireFrameの取得
//====================================================================

TakeC::WireFrame* TakeCFrameWork::GetWireFrame() {
	assert(wireFrame_ && "WireFrameが生成されていません");
	return wireFrame_.get();
}

//====================================================================
//			LightManagerの取得
//====================================================================

TakeC::LightManager* TakeCFrameWork::GetLightManager() {
	assert(lightManager_ && "LightManagerが生成されていません");
	return lightManager_.get();
}

//====================================================================
//			SpriteManagerの取得
//====================================================================
TakeC::SpriteManager* TakeCFrameWork::GetSpriteManager() {
	assert(spriteManager_ && "SpriteManagerが生成されていません");
	return spriteManager_.get();
}

//====================================================================
//			UIManagerの取得
//====================================================================
TakeC::UIManager* TakeCFrameWork::GetUIManager() {
	assert(uiManager_ && "UIManagerが生成されていません");
	return uiManager_.get();
}

//====================================================================
//			EventManagerの取得
//====================================================================
TakeC::EventManager* TakeCFrameWork::GetEventManager() {
	assert(eventManager_ && "EventManagerが生成されていません");
	return eventManager_.get();
}
