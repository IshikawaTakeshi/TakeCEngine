#include "GamePlayScene.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include "Vector3Math.h"
#include "ImGuiManager.h"
#include <format>
#include <numbers>
//====================================================================
//			初期化
//====================================================================

void GamePlayScene::Initialize() {
	//サウンドデータ
	soundData1 = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/fanfare.wav");

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera0_->SetTranslate({ 0.0f,4.0f,-20.0f });
	camera0_->SetRotate({ 0.16f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera0", *camera0_);

	//Camera1
	camera1_ = std::make_shared<Camera>();
	camera1_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera1_->SetTranslate({ 5.0f,0.0f,-1.0f });
	camera1_->SetRotate({ 0.0f,-1.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera1", *camera1_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	//Model読み込み
	ModelManager::GetInstance()->LoadModel("gltf","cube.gltf");
	ModelManager::GetInstance()->LoadModel("gltf","plane.gltf");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend","axis.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "terrain.obj");

	//Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/images/uvChecker.png");

	//Object3d
	object3d = std::make_shared<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance(), "terrain.obj");

	object3d1 = std::make_shared<Object3d>();
	object3d1->Initialize(Object3dCommon::GetInstance(), "plane.gltf");

	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "Plane", "plane.obj");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "Sphere", "sphere.obj");
	particleEmitter1_ = std::make_unique<ParticleEmitter>();
	particleEmitter2_ = std::make_unique<ParticleEmitter>();
	particleEmitter1_->Initialize("Emitter1",{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{3.0f,0.0f,0.0f} },30, 0.5f);
	particleEmitter2_->Initialize("Emitter2",{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{-3.0f,0.0f,0.0f} },30, 0.5f);
	particleEmitter1_->SetParticleName("Plane");
	particleEmitter2_->SetParticleName("Sphere");

	//MT4
	rotation0 = QuaternionMath::MakeRotateAxisAngleQuaternion({ 0.71f,0.71f,0.0f }, 0.3f);
	rotation1 = { -rotation0.x,-rotation0.y,-rotation0.z,-rotation0.w };

	interpolate0 = QuaternionMath::Slerp(rotation0, rotation1, 0.0f);
	interpolate1 = QuaternionMath::Slerp(rotation0, rotation1, 0.3f);
	interpolate2 = QuaternionMath::Slerp(rotation0, rotation1, 0.5f);
	interpolate3 = QuaternionMath::Slerp(rotation0, rotation1, 0.7f);
	interpolate4 = QuaternionMath::Slerp(rotation0, rotation1, 1.0f);
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	sprite_.reset();    //スプライトの解放

	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	//AudioManager::GetInstance()->SoundUnload(&soundData1); //音声データ解放
}

//====================================================================
//			更新処理
//====================================================================

void GamePlayScene::Update() {
	//ImGuiの更新
#ifdef _DEBUG
	CameraManager::GetInstance()->UpdateImGui();
	//sprite_->UpdateImGui(0);
	//Object3dCommon::GetInstance()->UpdateImGui();
	//particleEmitter1_->UpdateImGui();
	//particleEmitter2_->UpdateImGui();
	//TakeCFrameWork::GetParticleManager()->UpdateImGui();

	ImGui::Begin("MT4");
	ImGuiManager::QuaternionScreenPrintf("interpolate0", interpolate0);
	ImGuiManager::QuaternionScreenPrintf("interpolate1", interpolate1);
	ImGuiManager::QuaternionScreenPrintf("interpolate2", interpolate2);
	ImGuiManager::QuaternionScreenPrintf("interpolate3", interpolate3);
	ImGuiManager::QuaternionScreenPrintf("interpolate4", interpolate4);
	ImGui::End();
	

#endif // DEBUG


	//カメラの更新
	CameraManager::GetInstance()->Update();

	sprite_->Update(); 	//Spriteの更新
	//3Dオブジェクトの更新
	object3d->Update(); 
	object3d1->Update();

	//パーティクル発生器の更新
	particleEmitter1_->Update(); 
	particleEmitter2_->Update();

	TakeCFrameWork::GetParticleManager()->Update(); //パーティクルの更新

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();     //Spriteの描画前処理
	sprite_->Draw();              //スプライトの描画

	Object3dCommon::GetInstance()->PreDraw();   //Object3dの描画前処理
	object3d->Draw();             //3Dオブジェクトの描画
	object3d1->Draw();

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画
}