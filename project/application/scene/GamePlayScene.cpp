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

	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "plane.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "sphere.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "skyBox.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "ground.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "axis.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend", "cube.obj");

	//Animation読み込み
	TakeCFrameWork::GetAnimator()->LoadAnimation("running.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Idle.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("throwAttack.gltf");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });
	//animationModel
	drawTestModel_ = std::make_unique<Object3d>();
	drawTestModel_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");
	drawTestModel_->SetPosition({ 0.0f,0.0f,0.0f });
	drawTestModel_->GetModel()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("running.gltf"));

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "Resources/images/rick.png");

	//CreateParticle
	//TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"PlayerBullet", "sphere.obj");
	//TakeCFrameWork::GetParticleManager()->SetParticleAttribute("PlayerBullet");

	//SampleCharacter
	sampleCharacter_ = std::make_unique<SampleCharacter>();
	sampleCharacter_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	CollisionManager::GetInstance()->Finalize();           // 当たり判定の解放
	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	//AudioManager::GetInstance()->SoundUnload(&BGM); //音声データ解放
}

//====================================================================
//			更新処理
//====================================================================
void GamePlayScene::Update() {
#ifdef _DEBUG
	drawTestModel_->UpdateImGui(0);
	
	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();

#endif // DEBUG

	//カメラの更新
	CameraManager::GetInstance()->Update();
	drawTestModel_->Update();

	//SkyBoxの更新
	skyBox_->Update();
	
	// プレイヤーの更新
	//player_->Update();

	sprite_->Update();

	//SampleCharacter
	sampleCharacter_->Update();

	//当たり判定の更新
	CheckAllCollisions();

  //パーティクルの更新
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("TITLE");
		// シーン切り替え依頼
		//AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	} else if (Input::GetInstance()->TriggerKey(DIK_O)) {
		//AudioManager::GetInstance()->SoundUnload(&BGM);
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw();    //天球の描画

	SpriteCommon::GetInstance()->PreDraw();    //スプライトの描画前処理
	sprite_->Draw();    //スプライトの描画
	
	Object3dCommon::GetInstance()->DisPatch();
	drawTestModel_->DisPatch();
	//Object3dの描画前処理
	Object3dCommon::GetInstance()->PreDraw();
	drawTestModel_->Draw();
	sampleCharacter_->Draw();

	CollisionManager::GetInstance()->PreDraw();   //当たり判定の描画前処理
	sampleCharacter_->DrawCollider(); //当たり判定の描画
	
	//ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	//TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画
}

void GamePlayScene::CheckAllCollisions() {

	//CollisionManager::GetInstance()->ClearCollider();

	//CollisionManager::GetInstance()->CheckAllCollisions();
}