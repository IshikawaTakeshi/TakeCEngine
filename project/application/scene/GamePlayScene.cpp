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
	BGM = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/gamePlayBGM.wav");

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
	ModelManager::GetInstance()->LoadModel("gltf", "enemy.gltf");
	ModelManager::GetInstance()->LoadModel("gltf", "enemyBullet.gltf");

	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "skyBox.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "ground.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "axis.obj");
	
	//Animation読み込み
	TakeCFrameWork::GetAnimator()->LoadAnimation("running.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Idle.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("throwAttack.gltf");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	// Player
	player_ = std::make_unique<Player>();
	player_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");
	player_->GetModel()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("Idle.gltf"));

	// Enemy
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(Object3dCommon::GetInstance(), "enemy.gltf",player_.get());
		
	//Ground
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(Object3dCommon::GetInstance(), "ground.obj");

	//HPBar
	playerhpBar_ = std::make_unique<HPBar>();
	playerhpBar_->Initialize(SpriteCommon::GetInstance(),
		"Resources/images/backHp.png",
		"Resources/images/flontHp.png","Resources/images/white.png");
	playerhpBar_->SetPosition({ 93,670 });
	playerhpBar_->SetSize({ 550,40 });
	playerhpBar_->SetbugePosition({ 9,644 });
	playerhpBar_->SetbugeSize({ 75,75 });

	enemyhpBar_ = std::make_unique<HPBar>();
	enemyhpBar_->Initialize(SpriteCommon::GetInstance(),
		"Resources/images/backHp.png",
		"Resources/images/flontHp.png", "Resources/images/white.png");
	enemyhpBar_->SetPosition({ 723,17 });
	enemyhpBar_->SetSize({ 550,40 });
	enemyhpBar_->SetbugePosition({ 647,1 });
	enemyhpBar_->SetbugeSize({ 75,75 });

	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"PlayerBullet", "sphere.obj");
	TakeCFrameWork::GetParticleManager()->SetParticleAttribute("PlayerBullet");
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	CollisionManager::GetInstance()->Finalize();           // 当たり判定の解放
	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	AudioManager::GetInstance()->SoundUnload(&BGM); //音声データ解放
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
	//particleEmitter2_->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

#endif // DEBUG

	// オーディオ再生
	if (!isSoundPlay) {
		AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), BGM,0.1f);
		isSoundPlay = true;
	}

	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();
	
	// プレイヤーの更新
	player_->Update();

	//敵の更新
	enemy_->Update();

	//地面の更新
	ground_->Update();

	//HPBarの更新
	playerhpBar_->Update(float(player_->GetHP()), float(player_->GetMaxHP()));
	enemyhpBar_->Update(float(enemy_->GetHP()), float(enemy_->GetMaxHP()));

  //パーティクルの更新
	TakeCFrameWork::GetParticleManager()->Update(); 

	CollisionManager::GetInstance()->ClearCollider();
	CheckAllCollisions();

	//シーン遷移
	if (!enemy_->GetIsAlive()) {
		AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	} else if (player_->GetHP() <= 0) {
		AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	}

#ifdef _DEBUG
	// デバッグ用:シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_P)) {
		// シーン切り替え依頼
		AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	} else if (Input::GetInstance()->TriggerKey(DIK_O)) {
		AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
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
	//HPBarの描画
	playerhpBar_->Draw();
	enemyhpBar_->Draw();

	//Object3dの描画前処理
	Object3dCommon::GetInstance()->PreDrawForObject3d();

	player_->DrawBullet();

	enemy_->Draw();

	ground_->Draw();


	// ディスパッチ
	Object3dCommon::GetInstance()->DisPatch();
	player_->DisPatch();

	//SkinningObject3dの描画前処理
	Object3dCommon::GetInstance()->PreDrawForSkinningObject3d();
	player_->Draw();    //プレイヤーの描画

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画
}

void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearCollider();

	const std::list<PlayerBullet*>& playerBullets = player_->GetBullet();

	CollisionManager::GetInstance()->RegisterCollider(player_.get());

	for (PlayerBullet* pBullet : playerBullets) {
		CollisionManager::GetInstance()->RegisterCollider(pBullet);
	}

	CollisionManager::GetInstance()->RegisterCollider(enemy_.get());

	const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullet();

	for (EnemyBullet* eBullet : enemyBullets) {
		CollisionManager::GetInstance()->RegisterCollider(eBullet);
	}


	CollisionManager::GetInstance()->CheckAllCollisions();
}