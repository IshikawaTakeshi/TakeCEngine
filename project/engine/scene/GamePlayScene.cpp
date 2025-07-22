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
	gameCamera_ = std::make_shared<Camera>();
	gameCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	gameCamera_->SetIsDebug(false);
	gameCamera_->SetTranslate({ 5.0f,0.0f,-10.0f });
	gameCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("gameCamera", *gameCamera_);

	//Camera1
	debugCamera_ = std::make_shared<Camera>();
	debugCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	debugCamera_->SetTranslate({ 5.0f,0.0f,-1.0f });
	debugCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	debugCamera_->SetIsDebug(true);
	CameraManager::GetInstance()->AddCamera("debugCamera", *debugCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

#pragma region CreateParticle
	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"BulletLight.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "CrossEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"DamageSpark.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "SmokeEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "SparkExplosion.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "ItemPointEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "WalkSmoke1.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "WalkSmoke2.json");

#pragma endregion

	//levelObjectの初期化
	levelObjects_ = std::move(sceneManager_->GetLevelObjects());

	for (auto& object : levelObjects_) {
		object.second->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
	}

	//Animation読み込み
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation","walk.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation","Idle.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation","running.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("Animation","throwAttack.gltf");
	TakeCFrameWork::GetAnimator()->LoadAnimation("gltf","player_singleMesh.gltf");

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_pool.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

	//BulletManager
	bulletManager_ = std::make_unique<BulletManager>();
	bulletManager_->Initialize(Object3dCommon::GetInstance(), 100); //弾の最大数:100

	//player
	player_ = std::make_unique<Player>();
	player_->Initialize(Object3dCommon::GetInstance(), "player_singleMesh.gltf");
	player_->WeaponInitialize(Object3dCommon::GetInstance(), bulletManager_.get());
	player_->GetObject3d()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("player_singleMesh.gltf", "moveshot"));
	player_->SetTranslate({ 0.0f, 0.0f, -30.0f });
	playerHpBar_ = std::make_unique<HPBar>();
	playerHpBar_->Initialize(SpriteCommon::GetInstance(), "black.png", "flontHp.png");
	playerHpBar_->SetSize({ 200.0f, 10.0f }); // HPバーのサイズ
	playerHpBar_->SetPosition({ 50.0f, 500.0f }); // HPバーの位置
	//Enemy
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(Object3dCommon::GetInstance(), "player_singleMesh.gltf");
	enemy_->WeaponInitialize(Object3dCommon::GetInstance(), bulletManager_.get());
	enemy_->GetObject3d()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("player_singleMesh.gltf", "moveshot"));
	enemyHpBar_ = std::make_unique<HPBar>();
	enemyHpBar_->Initialize(SpriteCommon::GetInstance(), "black.png", "flontHp.png");
	enemyHpBar_->SetSize({ 500.0f, 10.0f }); // HPバーのサイズ
	enemyHpBar_->SetPosition({ 250.0f, 35.0f }); // HPバーの位置
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	CollisionManager::GetInstance()->ClearGameCharacter(); // 当たり判定の解放
	CameraManager::GetInstance()->ResetCameras(); //カメラのリセット
	player_.reset();
	skyBox_.reset();
}

//====================================================================
//			更新処理
//====================================================================
void GamePlayScene::Update() {

	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();
	
	//enemy
	enemy_->SetFocusTargetPos(player_->GetObject3d()->GetTranslate());
	enemy_->Update();

	//player
	player_->SetFocusTargetPos(enemy_->GetObject3d()->GetTranslate());
	player_->Update();
	//playerのHPバーの更新
	playerHpBar_->Update(player_->GetHealth(), player_->GetMaxHealth());
	//enemyのHPバーの更新
	enemyHpBar_->Update(enemy_->GetHealth(), enemy_->GetMaxHealth());

	//弾の更新
	bulletManager_->Update();

	for (auto& object : levelObjects_) {
		object.second->Update();
	}

	//particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();

	//当たり判定の更新
	CheckAllCollisions();


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

void GamePlayScene::UpdateImGui() {

	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();
	ParticleCommon::GetInstance()->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

	player_->UpdateImGui();
	enemy_->UpdateImGui();
	playerHpBar_->UpdateImGui("player");
	enemyHpBar_->UpdateImGui("enemy");
	ImGui::Begin("Level Objects");
	for(auto& object : levelObjects_) {
		object.second->UpdateImGui();
	}
	ImGui::End();
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw();    //天球の描画

#pragma region Object3d描画

	//Object3dの描画前処理
	Object3dCommon::GetInstance()->DisPatch();
	player_->GetObject3d()->DisPatch();
	enemy_->GetObject3d()->DisPatch();

	//for (auto& object : levelObjects_) {
	//	object->DisPatch();
	//}

	Object3dCommon::GetInstance()->PreDraw();
	player_->Draw();
	enemy_->Draw();
	bulletManager_->Draw();
	for (auto& object : levelObjects_) {
		object.second->Draw();
	}

#pragma endregion

	//当たり判定の描画前処理
	player_->DrawCollider();
	enemy_->DrawCollider();
	bulletManager_->DrawCollider();
	for (auto& object : levelObjects_) {
		object.second->DrawCollider();
	}

	//グリッド地面の描画
	TakeCFrameWork::GetWireFrame()->DrawGridGround({ 0.0f,0.0f,0.0f }, { 1000.0f, 1000.0f, 1000.0f }, 50);
	TakeCFrameWork::GetWireFrame()->DrawGridBox({
		{-500.0f,-500.0f,-500.0f},{500.0f,500.0f,500.0f } }, 2);
	TakeCFrameWork::GetWireFrame()->Draw();

	ParticleCommon::GetInstance()->PreDraw();   //パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); //パーティクルの描画


#pragma region スプライト描画
	//スプライトの描画前処理
	SpriteCommon::GetInstance()->PreDraw();
	//HPバーの描画
	playerHpBar_->Draw(); //プレイヤーのHPバーの描画
	enemyHpBar_->Draw(); //敵のHPバーの描画
#pragma endregion

	//GPUパーティクルの描画
	//ParticleCommon::GetInstance()->PreDrawForGPUParticle();

}

void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearGameCharacter();

	const std::vector<Bullet*>& bullets = bulletManager_->GetAllBullets();

	const std::vector<VerticalMissile*>& missiles = bulletManager_->GetAllMissiles();

	// プレイヤーの登録
	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(player_.get()));
	// 敵キャラクターの登録
	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(enemy_.get()));

	//弾の登録
	for (const auto& bullet : bullets) {
		if (bullet->GetIsActive()) {
			CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(bullet));
		}
	}
	//垂直ミサイルの登録
	for( const auto& missile : missiles) {
		if (missile->GetIsActive()) {
			CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(missile));
		}
	}

	// レベルオブジェクトの登録
	for (const auto& object : levelObjects_) {
		CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(object.second.get()));
	}


	CollisionManager::GetInstance()->CheckAllCollisionsForGameCharacter();
}