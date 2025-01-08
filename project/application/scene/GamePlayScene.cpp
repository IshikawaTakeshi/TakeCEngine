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
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "skyBox.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "ground.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "axis.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "bunny.obj");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	//Object3d
	/*object3d = std::make_unique<Object3d>();
	object3d->Initialize(Object3dCommon::GetInstance(), "plane.obj");

	humanObject = std::make_unique<Object3d>();
	humanObject->Initialize(Object3dCommon::GetInstance(), "walk.gltf");
	humanObject->SetPosition({ 0.0f,1.0f,0.0f });*/

	// Player
	player_ = std::make_unique<Player>();
	player_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");

	// ground
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(Object3dCommon::GetInstance(), "ground.obj");


	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "Plane", "plane.obj");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "Sphere", "sphere.obj");
	particleEmitter1_ = std::make_unique<ParticleEmitter>();
	particleEmitter2_ = std::make_unique<ParticleEmitter>();
	particleEmitter1_->Initialize("Emitter1",{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{3.0f,0.0f,0.0f} },30, 0.5f);
	particleEmitter2_->Initialize("Emitter2",{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{-3.0f,0.0f,0.0f} },30, 0.5f);
	particleEmitter1_->SetParticleName("Plane");
	particleEmitter2_->SetParticleName("Sphere");
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
	//sprite_->UpdateImGui(0);
	//Object3dCommon::GetInstance()->UpdateImGui();
	//object3d->UpdateImGui(0);
	//humanObject->UpdateImGui(2);
	player_->UpdateImGui();
	particleEmitter1_->UpdateImGui();
	particleEmitter2_->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

#endif // DEBUG


	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();

	//パーティクル発生器の更新
	particleEmitter1_->Update(); 
	particleEmitter2_->Update();

	// 地面の更新
	ground_->Update();
	// プレイヤーの更新
	player_->Update();

	TakeCFrameWork::GetParticleManager()->Update(); //パーティクルの更新

	CollisionManager::GetInstance()->ClearCollider();
	CheckAllCollisions();

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

	//if (!enemy_->GetIsAlive()) {
	//	//AudioManager::GetInstance()->SoundUnload(&gamePlayBGM);
	//	SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	//} else if (player_->GetHP() <= 0) {
	//	//AudioManager::GetInstance()->SoundUnload(&gamePlayBGM);
	//	SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	//}
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	//SkyBoxの描画
	skyBox_->Draw();

	SpriteCommon::GetInstance()->PreDraw();     //Spriteの描画前処理

	Object3dCommon::GetInstance()->PreDrawForObject3d();   //Object3dの描画前処理
	ground_->Draw();    //地面の描画
	player_->DrawBullet();
	Object3dCommon::GetInstance()->PreDrawForSkinningObject3d();   //Object3dの描画前処理
	player_->Draw();    //プレイヤーの描画
	

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画
}

void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearCollider();

	const std::list<PlayerBullet*>& playerBullets = player_->GetBullet();
	//const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullet();

	CollisionManager::GetInstance()->RegisterCollider(player_.get());
	//CollisionManager::GetInstance()->RegisterCollider(enemy_.get());

	for (PlayerBullet* pBullet : playerBullets) {
		CollisionManager::GetInstance()->RegisterCollider(pBullet);
	}
	//for (EnemyBullet* eBullet : enemyBullets) {
	//	CollisionManager::GetInstance()->RegisterCollider(eBullet);
	//}

	CollisionManager::GetInstance()->CheckAllCollisions();
}