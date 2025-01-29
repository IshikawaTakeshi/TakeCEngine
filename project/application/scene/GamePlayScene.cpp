#include "GamePlayScene.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include "Vector3Math.h"
#include "ImGuiManager.h"
#include "Collision/CollisionManager.h"
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
	camera0_->SetTranslate({ 0.0f,20.0f,-50.0f });
	camera0_->SetRotate({ 0.15f,0.0f,0.0f });
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
	ModelManager::GetInstance()->LoadModel("gltf","walk.gltf");
	ModelManager::GetInstance()->LoadModel("gltf", "plane.gltf");
	ModelManager::GetInstance()->LoadModel("gltf", "running.gltf");
	ModelManager::GetInstance()->LoadModel("gltf/Animation_Node", "Animation_Node_00.gltf");
	ModelManager::GetInstance()->LoadModel("gltf/Animation_Node", "Animation_Node_01.gltf");
	ModelManager::GetInstance()->LoadModel("gltf/Animation_Node", "Animation_Node_02.gltf");
	ModelManager::GetInstance()->LoadModel("gltf/Animation_Node", "Animation_Node_03.gltf");
	ModelManager::GetInstance()->LoadModel("gltf/Animation_Node", "Animation_Node_04.gltf");
	ModelManager::GetInstance()->LoadModel("gltf/Animation_Node", "Animation_Node_05.gltf");

	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "skyBox.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "ground.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "axis.obj");
	//ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "bunny.obj");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	// Player
	player_ = std::make_unique<Player>();
	player_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/images/rick.png");

		
	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "Plane", "plane.obj");
	//TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "Sphere", "sphere.obj");
	particleEmitter1_ = std::make_unique<ParticleEmitter>();
	particleEmitter2_ = std::make_unique<ParticleEmitter>();
	particleEmitter1_->Initialize("Emitter1",{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{3.0f,0.0f,0.0f} },30, 0.5f);
	//particleEmitter2_->Initialize("Emitter2",{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{-3.0f,0.0f,0.0f} },30, 0.5f);
	particleEmitter1_->SetParticleName("Plane");
	//particleEmitter2_->SetParticleName("Sphere");
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	CollisionManager::GetInstance()->Finalize();           // 当たり判定の解放
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
	Object3dCommon::GetInstance()->UpdateImGui();

	player_->UpdateImGui();
	planeModel1_->UpdateImGui(1);
	planeModel2_->UpdateImGui(2);
	sphereModel_->UpdateImGui(3);
	AnimationModel_->UpdateImGui(4);
	particleEmitter1_->UpdateImGui();
	//particleEmitter2_->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

#endif // DEBUG

	sprite_->Update();

	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();
	
	// プレイヤーの更新
	player_->Update();


	particleEmitter1_->Update();
  //パーティクルの更新
	TakeCFrameWork::GetParticleManager()->Update(); 

	CollisionManager::GetInstance()->ClearCollider();
	CheckAllCollisions();

	//シーン遷移
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
#endif // _DEBUG
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	//SkyBoxの描画
	skyBox_->Draw();

	//Spriteの描画前処理
	SpriteCommon::GetInstance()->PreDraw();
	sprite_->Draw();    //スプライトの描画

	//Object3dの描画前処理
	Object3dCommon::GetInstance()->PreDrawForObject3d();

	player_->DrawBullet();

	// ディスパッチ
	//Object3dCommon::GetInstance()->DisPatch();
	//player_->DisPatch();

	//SkinningObject3dの描画前処理
	Object3dCommon::GetInstance()->PreDrawForSkinningObject3d();
	player_->Draw();    //プレイヤーの描画
	planeModel1_->Draw();
	planeModel2_->Draw();
	sphereModel_->Draw();
	AnimationModel_->Draw();

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	//TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画
}

void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearCollider();

	const std::list<PlayerBullet*>& playerBullets = player_->GetBullet();

	CollisionManager::GetInstance()->RegisterCollider(player_.get());

	for (PlayerBullet* pBullet : playerBullets) {
		CollisionManager::GetInstance()->RegisterCollider(pBullet);
	}


	CollisionManager::GetInstance()->CheckAllCollisions();
}