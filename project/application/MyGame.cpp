#include "MyGame.h"
#include "scene/SceneFactory.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "Collision/CollisionManager.h"

float MyGame::requestedTimeScale_ = 1.0f;
Timer MyGame::timeScaleTimer_;

//====================================================================
//			初期化
//====================================================================

void MyGame::Initialize(const std::wstring& titleName) {

	using Clock = std::chrono::high_resolution_clock;

	// 計測開始
	auto start = Clock::now();

	//FrameWorkの初期化
	TakeCFrameWork::Initialize(titleName);

	sceneFactory_ = std::make_unique<SceneFactory>();

	//シーンマネージャーのセット
	SceneManager::GetInstance().SetSceneFactory(sceneFactory_.get());

	//影描画用レンダーテクスチャの生成
	shadowRenderTexture_ = std::make_unique<RenderTexture>();
	shadowRenderTexture_->Initialize(directXCommon_.get(), srvManager_.get(),
		1024 * 4, 1024 * 4
	);
	//PostEffectManagerに影描画用レンダーテクスチャをセット
	postEffectManager_->SetLightCameraRenderTexture(shadowRenderTexture_.get());

	//Texture読み込み
	TakeC::TextureManager::GetInstance().LoadTextureAll();
	//Model読み込み
	LoadModel();
	//Animation読み込み
	LoadAnimation();
	//Sound読み込み
	LoadSound();
	//ParticlePreset読み込み
	LoadParticlePreset();
	

	//postEffectManager_->InitializeEffect("Vignette",    L"PostEffect/Vignette.CS.hlsl");
	//postEffectManager_->InitializeEffect("GrayScale",   L"PostEffect/GrayScale.CS.hlsl");
	postEffectManager_->InitializeEffect("Dissolve",    L"PostEffect/Dissolve.CS.hlsl");
	postEffectManager_->InitializeEffect("RadialBluer", L"PostEffect/RadialBlur.CS.hlsl");
	//postEffectManager_->InitializeEffect("BoxFilter",   L"PostEffect/BoxFilter.CS.hlsl");
	postEffectManager_->InitializeEffect("BloomEffect", L"PostEffect/BloomEffect.CS.hlsl");
	//postEffectManager_->InitializeEffect("LuminanceBasedOutline", L"PostEffect/LuminanceBasedOutline.CS.hlsl");
	postEffectManager_->InitializeEffect("DepthBasedOutline", L"PostEffect/DepthBasedOutline.CS.hlsl");
	postEffectManager_->InitializeEffect("ShadowMapEffect",   L"PostEffect/ShadowMapEffect.CS.hlsl");

	CollisionManager::GetInstance().Initialize(directXCommon_.get());

	//最初のシーンを設定
#ifdef _DEBUG

	sceneManager_->ChangeScene("TITLE",0.0f);
#else
	sceneManager_->ChangeScene("TITLE", 0.0f);
#endif // _DEBUG


	// 計測終了
	auto end = Clock::now();

	// 経過時間をミリ秒単位で計算
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	Logger::Log(StringUtility::ConvertString(L"Resource Load Time : " + std::to_wstring(duration) + L"ms"));
}

//====================================================================
//			終了処理
//====================================================================

void MyGame::Finalize() {
	TakeCFrameWork::GetParticleManager()->Finalize(); //パーティクルマネージャーの解放
	CollisionManager::GetInstance().Finalize();
	sceneTransition_->Finalize(); //シーン遷移の開放
	sceneManager_->Finalize();  //シーンの開放
	TakeCFrameWork::Finalize(); //FrameWorkの終了処理
}

//====================================================================
//			更新処理
//====================================================================

void MyGame::Update() {

	timeScaleTimer_.Update(requestedTimeScale_);

	if (timeScaleTimer_.IsFinished() == false) {

		timeScale_ = std::clamp(timeScaleTimer_.GetEase(Easing::EasingType::OUT_QUAD), 0.2f, 1.0f);
	}


	//FrameWorkの更新
	TakeCFrameWork::Update();
	TakeCFrameWork::GetWireFrame()->Update();
}

//====================================================================
//			描画処理
//====================================================================

void MyGame::Draw() {
	
	//===========================================
	// 1. シャドウパス
	//===========================================
	shadowRenderTexture_->ClearRenderTarget();
	srvManager_->SetDescriptorHeap();
	sceneManager_->DrawShadow();  // ライトカメラ視点で深度のみ描画

	//===========================================
	// 2. メインパス（シーン描画）
	//===========================================
	renderTexture_->ClearRenderTarget();
	srvManager_->SetDescriptorHeap();
	sceneManager_->DrawObject();  // 通常のオブジェクト描画
	sceneManager_->DrawSprite();  // スプライト描画

	

	//===========================================
	// 3. ポストエフェクト
	//===========================================
	// シャドウマップ + メインカラー + 深度 を使って影を適用
	postEffectManager_->AllDispatch();

	//===========================================
	// 4. 最終描画（スワップチェーンへ）
	//===========================================
	directXCommon_->PreDraw();

	renderTexture_->PreDraw();
	postEffectManager_->Draw(renderTexture_->GetRenderTexturePSO());
	renderTexture_->Draw();
	renderTexture_->PostDraw();

#ifdef _DEBUG
	imguiManager_->PostDraw();
#endif

	directXCommon_->PostDraw();

	//===========================================
	// 5. 次フレーム準備
	//===========================================

	// モデルのリロード適用
	TakeC::ModelManager::GetInstance().ApplyModelReloads();
}

void MyGame::RequestTimeScale(float timeScale, float duration,float current) {
	requestedTimeScale_ = timeScale;
	timeScaleTimer_.Initialize(duration, current);
}

//====================================================================
//			モデルの読み込み
//====================================================================

void MyGame::LoadModel() {
	//gltf
	TakeC::ModelManager::GetInstance().LoadModel("Bazooka.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("BazookaBullet.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("boostEffectCone.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Bullet.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("cube.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Deer.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Floor.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Ground.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Missile.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("plane.gltf","skyBox_blueSky.dds");
	TakeC::ModelManager::GetInstance().LoadModel("player_singleMesh.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("player_MultiMesh.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Player_Model_Ver2.0.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Enemy.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("ICOBall.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Rifle.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("VerticalMissileLauncher.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("ShotGun.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("MachineGun.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Rail.gltf");
	//TakeC::ModelManager::GetInstance().LoadModelAll();
	//obj

}

//====================================================================
//			アニメーションの読み込み
//====================================================================

void MyGame::LoadAnimation() {

	TakeCFrameWork::GetAnimationManager()->LoadAnimation("Player_Model_Ver2.0.gltf");
	//TakeCFrameWork::GetAnimator()->LoadAnimation("running.gltf");
	//TakeCFrameWork::GetAnimator()->LoadAnimation("throwAttack.gltf");
}

//====================================================================
//			テクスチャの読み込み
//====================================================================
void MyGame::LoadTexture() {

	TakeC::TextureManager::GetInstance().LoadTexture("UI/TitleText.png",false);
	TakeC::TextureManager::GetInstance().LoadTexture("UI/GameClearText.png",false);
	TakeC::TextureManager::GetInstance().LoadTexture("UI/GameOverText.png", false);
	TakeC::TextureManager::GetInstance().LoadTexture("UI/reticle_focusTarget.png", false);
	TakeC::TextureManager::GetInstance().LoadTexture("UI/numText.png", false);
}

//====================================================================
//			サウンドの読み込み
//====================================================================
void MyGame::LoadSound() {

}

//====================================================================
//			パーティクルプリセットの読み込み
//====================================================================
void MyGame::LoadParticlePreset() {

	TakeCFrameWork::GetParticleManager()->LoadAllPresets();
}
