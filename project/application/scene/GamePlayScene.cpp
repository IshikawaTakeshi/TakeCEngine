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

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera0_->SetTranslate({ 0.0f,75.0f,-20.0f });
	camera0_->SetRotate({ 1.0f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Camera0", *camera0_);


	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	//Model読み込み
	ModelManager::GetInstance()->LoadModel("gltf","walk.gltf");
	//ModelManager::GetInstance()->LoadModel("obj_mtl_blend","axis.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "terrain.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "lock.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "Player.obj");

	//Sprite
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/images/uvChecker.png");

	//player
	player_ = std::make_unique<Player>();
	player_->Initialize(Object3dCommon::GetInstance(), "Player.obj");

	//enemy
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(Object3dCommon::GetInstance(), "sphere.obj");

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

	ImGui::Begin("How to use");
	ImGui::Text("Camera");
	ImGui::Text("Rotate : Mouse Right Button Drag");
	ImGui::Text("MoveXY : Mouse Middle Button Drag");
	ImGui::Text("MoveZ : Mouse Wheel");
	ImGui::End();

#endif // DEBUG


	//カメラの更新
	CameraManager::GetInstance()->Update();

	//sprite_->Update(); 	//Spriteの更新
	//3Dオブジェクトの更新
	player_->Update(); 
	enemy_->Update();
	//パーティクル発生器の更新

	CollisionManager::GetInstance()->ClearCollider();
	CheckAllCollisions();

	//シーン遷移
	if (!enemy_->GetIsAlive()) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();     //Spriteの描画前処理
	sprite_->Draw();              //スプライトの描画

	Object3dCommon::GetInstance()->PreDrawForObject3d();   //Object3dの描画前処理
	player_->Draw();             //3Dオブジェクトの描画
	enemy_->Draw();
	Object3dCommon::GetInstance()->PreDrawForSkinningObject3d();   //Object3dの描画前処理

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画
}


void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearCollider();

	const std::list<PlayerBullet*>& playerBullets = player_->GetBullet();
	const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullet();

	CollisionManager::GetInstance()->RegisterCollider(player_.get());
	CollisionManager::GetInstance()->RegisterCollider(enemy_.get());

	for (PlayerBullet* pBullet : playerBullets) {
		CollisionManager::GetInstance()->RegisterCollider(pBullet);
	}
	for (EnemyBullet* eBullet : enemyBullets) {
		CollisionManager::GetInstance()->RegisterCollider(eBullet);
	}

	CollisionManager::GetInstance()->CheckAllCollisions();
}