#include "MyGame.h"
#include "scene/SceneFactory.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "Collision/CollisionManager.h"
#include "math/MathEnv.h"

float MyGame::requestedTimeScale_ = 1.0f;
Timer MyGame::timeScaleTimer_;

//====================================================================
//			初期化
//====================================================================

void MyGame::Initialize(const std::wstring& titleName) {


	using Clock = std::chrono::high_resolution_clock;
	using Ms = std::chrono::milliseconds;

	// 計測開始（全体）
	auto totalStart = Clock::now();

	//FrameWorkの初期化
	TakeCFrameWork::Initialize(titleName);

	sceneFactory_ = std::make_unique<SceneFactory>();

	//シーンマネージャーのセット
	SceneManager::GetInstance().SetSceneFactory(sceneFactory_.get());

	//影描画用レンダ���テクスチャの生成
	shadowRenderTexture_ = std::make_unique<RenderTexture>();
	shadowRenderTexture_->Initialize(directXCommon_.get(), srvManager_.get(),
		1024 * 4, 1024 * 4
	);
	//PostEffectManagerに影描画用レンダーテクスチャをセット
	postEffectManager_->SetLightCameraRenderTexture(shadowRenderTexture_.get());

	//----------------------------------------
	// リソース読み��み時間計測
	//----------------------------------------

	int64_t textureMs = Measure(L"TextureManager::LoadTextureAll", []() {
		TakeC::TextureManager::GetInstance().LoadTextureAll();
		});

	int64_t modelMs = Measure(L"MyGame::LoadModel", [this]() {
		LoadModel();
		});

	int64_t animationMs = Measure(L"MyGame::LoadAnimation", [this]() {
		LoadAnimation();
		});

	int64_t soundMs = Measure(L"MyGame::LoadSound", [this]() {
		LoadSound();
		});

	int64_t particlePresetMs = Measure(L"MyGame::LoadParticlePreset", [this]() {
		LoadParticlePreset();
		});

	int64_t postEffectPresetMs = Measure(L"MyGame::LoadPostEffectPresets", [this]() {
		LoadPostEffectPresets();
		});

	// post effect 初期化も計測したい場合
	int64_t postEffectInitMs = Measure(L"PostEffect InitializeEffect", [this]() {
		//postEffectManager_->InitializeEffect("Vignette",    L"PostEffect/Vignette.CS.hlsl");
		//postEffectManager_->InitializeEffect("Dissolve", L"PostEffect/Dissolve.CS.hlsl");
		postEffectManager_->InitializeEffect("DistortionEffect", L"PostEffect/DistortionEffect.CS.hlsl");
		postEffectManager_->InitializeEffect("RadialBlur", L"PostEffect/RadialBlur.CS.hlsl");
		postEffectManager_->InitializeEffect("BloomEffect", L"PostEffect/BloomEffect.CS.hlsl");
		postEffectManager_->InitializeEffect("DepthBasedOutline", L"PostEffect/DepthBasedOutline.CS.hlsl");
		postEffectManager_->InitializeEffect("ShadowMapEffect", L"PostEffect/ShadowMapEffect.CS.hlsl");
		postEffectManager_->InitializeEffect("ScanlineEffect", L"PostEffect/ScanlineEffect.CS.hlsl");
		postEffectManager_->InitializeEffect("ChromaticAberration", L"PostEffect/ChromaticAberration.CS.hlsl");
		});

	CollisionManager::GetInstance().Initialize(directXCommon_.get());

	//最初のシーンを設定
#if defined(_DEBUG) || defined(_DEVELOP)
	sceneManager_->ChangeScene("TITLE", 0.0f);
#else
	sceneManager_->ChangeScene("TITLE", 0.0f);
#endif // _DEBUG

	// 計測終了（全体）
	auto totalEnd = Clock::now();
	auto totalMs = std::chrono::duration_cast<Ms>(totalEnd - totalStart).count();

	// サマリーログ
	Logger::Log(StringUtility::ConvertString(
		L"[LoadTime] Summary(ms) "
		L"Texture=" + std::to_wstring(textureMs) +
		L", Model=" + std::to_wstring(modelMs) +
		L", Animation=" + std::to_wstring(animationMs) +
		L", Sound=" + std::to_wstring(soundMs) +
		L", ParticlePreset=" + std::to_wstring(particlePresetMs) +
		L", PostEffectPreset=" + std::to_wstring(postEffectPresetMs) +
		L", PostEffectInit=" + std::to_wstring(postEffectInitMs) +
		L", Total=" + std::to_wstring(totalMs)
	));
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
	//PostEffectの更新
	postEffectManager_->Update();

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

#if defined(_DEBUG) || defined(_DEVELOP)
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
	TakeC::ModelManager::GetInstance().LoadModel("Floor.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Ground.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Missile.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("plane.gltf","skyBox_blueSky.dds");
	TakeC::ModelManager::GetInstance().LoadModel("Player_Model_Ver2.0.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Enemy_Model.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("ICOBall.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Rifle.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("Rail.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("VerticalMissileLauncher.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("ShotGun.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("MachineGun.gltf");
	TakeC::ModelManager::GetInstance().LoadModel("TestBox.gltf");
	//TakeC::ModelManager::GetInstance().LoadModelAll();
	//obj

}

//====================================================================
//			アニメーションの読み込み
//====================================================================

void MyGame::LoadAnimation() {

	TakeCFrameWork::GetAnimationManager()->LoadAnimation("Player_Model_Ver2.0.gltf");
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

//====================================================================
//			PostEffectプリセットの読み込み
//====================================================================
void MyGame::LoadPostEffectPresets() {
	postEffectManager_->LoadPresets();
}
