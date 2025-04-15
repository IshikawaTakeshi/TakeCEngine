#include "MyGame.h"
#include "scene/SceneFactory.h"
#include "Logger.h"
#include "StringUtility.h"
#include "Collision/CollisionManager.h"



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

	CollisionManager::GetInstance()->Initialize(directXCommon_.get());

	//最初のシーンを設定
	SceneManager::GetInstance()->ChangeScene("GAMEPLAY");

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
	sceneManager_->Finalize();  //シーンの開放
	TakeCFrameWork::Finalize(); //FrameWorkの終了処理
}

//====================================================================
//			更新処理
//====================================================================

void MyGame::Update() {

	//FrameWorkの更新
	TakeCFrameWork::Update();
	TakeCFrameWork::GetPrimitiveDrawer()->Update();
	TakeCFrameWork::GetWireFrame()->Update();
}

//====================================================================
//			描画処理
//====================================================================

void MyGame::Draw() {

	 //描画前処理
	directXCommon_->PreDraw();
	srvManager_->SetDescriptorHeap();       //SRV描画前処理
	
	sceneManager_->Draw();
	
	//描画後処理
	directXCommon_->PostDraw();
}

//====================================================================
//			モデルの読み込み
//====================================================================

void MyGame::LoadModel() {
	//gltf
	ModelManager::GetInstance()->LoadModel("gltf", "walk.gltf");
	ModelManager::GetInstance()->LoadModel("gltf", "plane.gltf");
	//obj
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "skyBox.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "ground.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "axis.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "cube.obj");
}

//====================================================================
//			アニメーションの読み込み
//====================================================================

void MyGame::LoadAnimation() {

	TakeCFrameWork::GetAnimator()->LoadAnimation("Idle.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("running.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("throwAttack.gltf");
}