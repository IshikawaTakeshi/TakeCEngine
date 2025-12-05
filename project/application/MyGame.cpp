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
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());

	//Model読み込み
	LoadModel();
	//Animation読み込み
	LoadAnimation();
	//Sound読み込み
	LoadSound();
	//ParticlePreset読み込み
	LoadParticlePreset();
	//Texture読み込み
	TextureManager::GetInstance()->LoadTextureAll();

	//postEffectManager_->InitializeEffect("Vignette",    L"PostEffect/Vignette.CS.hlsl");
	//postEffectManager_->InitializeEffect("GrayScale",   L"PostEffect/GrayScale.CS.hlsl");
	postEffectManager_->InitializeEffect("Dissolve",    L"PostEffect/Dissolve.CS.hlsl");
	postEffectManager_->InitializeEffect("RadialBluer", L"PostEffect/RadialBlur.CS.hlsl");
	//postEffectManager_->InitializeEffect("BoxFilter",   L"PostEffect/BoxFilter.CS.hlsl");
	postEffectManager_->InitializeEffect("BloomEffect", L"PostEffect/BloomEffect.CS.hlsl");
	//postEffectManager_->InitializeEffect("LuminanceBasedOutline", L"PostEffect/LuminanceBasedOutline.CS.hlsl");
	postEffectManager_->InitializeEffect("DepthBasedOutline",     L"PostEffect/DepthBasedOutline.CS.hlsl");

	CollisionManager::GetInstance()->Initialize(directXCommon_.get());

	//最初のシーンを設定
#ifdef _DEBUG

	SceneManager::GetInstance()->ChangeScene("TITLE",0.0f);
#else
	SceneManager::GetInstance()->ChangeScene("TITLE", 0.0f);
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
	CollisionManager::GetInstance()->Finalize();
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

	//renderTextureの描画前処理
	renderTexture_->PreDraw();
	//SRV描画前処理
	srvManager_->SetDescriptorHeap(); 
	//オブジェクト描画
	sceneManager_->DrawObject();
	//スプライト描画
	sceneManager_->DrawSprite();
	//postEffect計算処理
	postEffectManager_->AllDispatch();
	 //描画前処理
	directXCommon_->PreDraw();
	//RenderTexture描画
	renderTexture_->Draw();
	
	//renderTexture描画後処理
	renderTexture_->PostDraw();

#ifdef _DEBUG
	imguiManager_->PostDraw();
#endif
	//描画後処理
	directXCommon_->PostDraw();
	//モデルのリロード適用
	ModelManager::GetInstance()->ApplyModelReloads();
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
	ModelManager::GetInstance()->LoadModel("walk.gltf");
	ModelManager::GetInstance()->LoadModel("plane.gltf","skyBox_blueSky.dds");
	ModelManager::GetInstance()->LoadModel("player_singleMesh.gltf");
	ModelManager::GetInstance()->LoadModel("player_MultiMesh.gltf");
	ModelManager::GetInstance()->LoadModel("cube.gltf");
	ModelManager::GetInstance()->LoadModel("ICOBall.gltf");
	ModelManager::GetInstance()->LoadModel("Rifle.gltf");
	ModelManager::GetInstance()->LoadModel("Bazooka.gltf");
	ModelManager::GetInstance()->LoadModel("VerticalMissileLauncher.gltf");
	ModelManager::GetInstance()->LoadModel("MachineGun.gltf");
	ModelManager::GetInstance()->LoadModel("boostEffectCone.gltf");
	ModelManager::GetInstance()->LoadModel("MultiICO.gltf");
	ModelManager::GetInstance()->LoadModel("Deer.gltf");
	ModelManager::GetInstance()->LoadModel("Bullet.gltf");
	//obj
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");
	ModelManager::GetInstance()->LoadModel("skyBox_airport.obj","rostock_laage_airport_4k.dds");
	ModelManager::GetInstance()->LoadModel("skyBox_blueSky.obj","skyBox_blueSky.dds");
	ModelManager::GetInstance()->LoadModel("skyBox_pool.obj","pool_4k.dds");
	ModelManager::GetInstance()->LoadModel("ground.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("cube.obj","pool_4k.dds");
}

//====================================================================
//			アニメーションの読み込み
//====================================================================

void MyGame::LoadAnimation() {

	//TakeCFrameWork::GetAnimator()->LoadAnimation("Idle.gltf");
	//TakeCFrameWork::GetAnimator()->LoadAnimation("running.gltf");
	//TakeCFrameWork::GetAnimator()->LoadAnimation("throwAttack.gltf");
}

//====================================================================
//			テクスチャの読み込み
//====================================================================
void MyGame::LoadTexture() {

	TextureManager::GetInstance()->LoadTexture("UI/TitleText.png",false);
	TextureManager::GetInstance()->LoadTexture("UI/GameClearText.png",false);
	TextureManager::GetInstance()->LoadTexture("UI/GameOverText.png", false);
	TextureManager::GetInstance()->LoadTexture("UI/reticle_focusTarget.png", false);
	TextureManager::GetInstance()->LoadTexture("UI/numText.png", false);
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

	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BoostEffect2.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BoostEffect3.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BoostEffect4.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BulletLight.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "CrossEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "DamageSpark.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "SmokeEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "SparkExplosion.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "ItemPointEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "WalkSmoke1.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "WalkSmoke2.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "MissileSmoke.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "MissileExplosion.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "DeadExplosionEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "DeadSmokeEffect.json");
}
