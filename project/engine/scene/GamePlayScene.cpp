#include "GamePlayScene.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "Vector3Math.h"
#include "ImGuiManager.h"
#include "Collision/CollisionManager.h"
#include "MyGame.h"
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
	gameCamera_->SetRotate({ 0.0f,-1.4f,0.0f,1.0f });
	CameraManager::GetInstance()->AddCamera("gameCamera", *gameCamera_);

	//Camera1
	debugCamera_ = std::make_shared<Camera>();
	debugCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	debugCamera_->SetTranslate({ 5.0f,0.0f,-1.0f });
	debugCamera_->SetRotate({ 0.0f,-1.4f,0.0f,1.0f });
	debugCamera_->SetIsDebug(true);
	CameraManager::GetInstance()->AddCamera("debugCamera", *debugCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

#pragma region CreateParticle
	//CreateParticle
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BoostEffect2.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "BoostEffect3.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"BulletLight.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "CrossEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(),"DamageSpark.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "SmokeEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "SparkExplosion.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "ItemPointEffect.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "WalkSmoke1.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "WalkSmoke2.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "MissileSmoke.json");
	TakeCFrameWork::GetParticleManager()->CreateParticleGroup(ParticleCommon::GetInstance(), "MissileExplosion.json");

#pragma endregion

	//levelObjectの初期化
	sceneManager_->LoadLevelData("levelData_gameScene_3");
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
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_blueSky.obj");
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
	//Enemy
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(Object3dCommon::GetInstance(), "player_singleMesh.gltf");
	enemy_->WeaponInitialize(Object3dCommon::GetInstance(), bulletManager_.get());
	enemy_->GetObject3d()->SetAnimation(TakeCFrameWork::GetAnimator()->FindAnimation("player_singleMesh.gltf", "moveshot"));

	// playerHpBar
	playerHpBar_ = std::make_unique<HPBar>();
	playerHpBar_->Initialize(SpriteCommon::GetInstance(), "black.png", "flontHp.png");
	playerHpBar_->SetSize({ 200.0f, 10.0f }); // HPバーのサイズ
	playerHpBar_->SetPosition({ 50.0f, 500.0f }); // HPバーの位置
	// enemyHpBar
	enemyHpBar_ = std::make_unique<HPBar>();
	enemyHpBar_->Initialize(SpriteCommon::GetInstance(), "black.png", "flontHp.png");
	enemyHpBar_->SetSize({ 400.0f, 10.0f }); // HPバーのサイズ
	enemyHpBar_->SetPosition({ 300.0f, 35.0f }); // HPバーの位置
	//playerReticle
	playerReticle_ = std::make_unique<PlayerReticle>();
	playerReticle_->Initialize();
	//energyInfoUI
	energyInfoUI_ = std::make_unique<EnergyInfoUI>();
	energyInfoUI_->Initialize(SpriteCommon::GetInstance(), "black.png", "flontHp.png");
	energyInfoUI_->SetSize({ 400.0f, 10.0f }); // エネルギーUIのサイズ
	energyInfoUI_->SetPosition({ 300.0f, 525.0f }); // エネルギーUIの位置
	//bulletCounterUI
	bulletCounterUI_.resize(4); // 4つの弾数カウンターを用意
	bulletCounterUI_[0] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[0]->Initialize(SpriteCommon::GetInstance(), {760.0f,470.0f});
	bulletCounterUI_[1] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[1]->Initialize(SpriteCommon::GetInstance(), {900.0f,470.0f});
	bulletCounterUI_[2] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[2]->Initialize(SpriteCommon::GetInstance(), {760.0f,540.0f});
	bulletCounterUI_[3] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[3]->Initialize(SpriteCommon::GetInstance(), {900.0f,540.0f});

	//操作説明UI
	instructionSprite_ = std::make_unique<Sprite>();
	instructionSprite_->Initialize(SpriteCommon::GetInstance(), "UI/OperationInstructions.png");
	instructionSprite_->SetPosition({ 0.0f, 100.0f });
	instructionSprite_->AdjustTextureSize();
	instructionSprite_->SetSize({ 250.0f, 200.0f });
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {
	TakeCFrameWork::GetParticleManager()->Finalize(); //パーティクルマネージャーの解放
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

	//弾の更新
	bulletManager_->Update();

	for (auto& object : levelObjects_) {
		object.second->Update();
	}

	//particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();
	//particleEmitter_->UpdateForGPU();
	//particleEmitter_->EmitParticle(gpuParticle_.get());
	//gpuParticle_->Update();

	//当たり判定の更新
	CheckAllCollisions();

	if (behaviorRequest_) {

		behavior_ = behaviorRequest_.value();

		switch (behavior_) {
		case SceneBehavior::GAMESTART:

			InitializeGameStart();
			break;
		case SceneBehavior::GAMEPLAY:

			InitializeGamePlay();
			break;
		case SceneBehavior::ENEMYDESTROYED:
			InitializeEnemyDestroyed();
			break;
		case SceneBehavior::GAMEOVER:

			InitializeGameOver();
			break;
		case SceneBehavior::GAMECLEAR:

			InitializeGameClear();
			break;
		case SceneBehavior::PAUSE:

			InitializePause();
			break;
		default:
			break;
		}

		behaviorRequest_ = std::nullopt;
	}
	
	switch (behavior_) {
	case SceneBehavior::GAMESTART:

		UpdateGameStart();
		break;
	case SceneBehavior::GAMEPLAY:

		UpdateGamePlay();
		break;
	case SceneBehavior::ENEMYDESTROYED:
		UpdateEnemyDestroyed();
		break;
	case SceneBehavior::GAMEOVER:

		UpdateGameOver();
		break;
	case SceneBehavior::GAMECLEAR:

		UpdateGameClear();
		break;
	case SceneBehavior::PAUSE:

		UpdatePause();
		break;
	default:
		break;
	}
}

void GamePlayScene::UpdateImGui() {

	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();
	
	player_->UpdateImGui();
	enemy_->UpdateImGui();
	playerHpBar_->UpdateImGui("player");
	enemyHpBar_->UpdateImGui("enemy");
	playerReticle_->UpdateImGui();
	energyInfoUI_->UpdateImGui("player");
	for(int i = 0; i < 4; i++) {
		bulletCounterUI_[i]->UpdateImGui(std::format("bulletCounter{}", i));
	}
	instructionSprite_->UpdateImGui("instruction");
	ImGui::Begin("Level Objects");
	/*for(auto& object : levelObjects_) {
		object.second->UpdateImGui();
	}*/
	ImGui::End();

	//particleEmitter_->UpdateImGui();
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw();    //天球の描画

#pragma region Object3d描画

	//Object3dの描画前処理
	Object3dCommon::GetInstance()->Dispatch();
	player_->GetObject3d()->Dispatch();
	enemy_->GetObject3d()->Dispatch();

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
	Object3dCommon::GetInstance()->PreDrawAddBlend();
	player_->DrawBoostEffect();

#pragma endregion

	//当たり判定の描画前処理
	enemy_->DrawCollider();
	TakeCFrameWork::GetWireFrame()->Draw();

	//パーティクルの描画
	TakeCFrameWork::GetParticleManager()->Draw();

#pragma region スプライト描画
	//スプライトの描画前処理
	SpriteCommon::GetInstance()->PreDraw();

	//プレイヤーのレティクルの描画
	playerReticle_->Draw();
	//HPバーの描画
	playerHpBar_->Draw(); //プレイヤーのHPバーの描画
	enemyHpBar_->Draw();  //敵のHPバーの描画
	//エネルギーUIの描画
	energyInfoUI_->Draw();
	//弾カウンターUIの描画
	for(auto& bulletUI : bulletCounterUI_) {
		bulletUI->Draw();
	}

	//操作説明UIの描画
	instructionSprite_->Draw();
#pragma endregion

	

}

//====================================================================
// ゲームスタート時の処理
//====================================================================

void GamePlayScene::InitializeGameStart() {}

void GamePlayScene::UpdateGameStart() {}

//====================================================================
// ゲームプレイ時の処理
//====================================================================
void GamePlayScene::InitializeGamePlay() {}

void GamePlayScene::UpdateGamePlay() {

	//playerReticleの更新
	playerReticle_->Update(player_->GetFocusTargetPos());

	//playerのHPバーの更新
	playerHpBar_->Update(player_->GetHealth(), player_->GetMaxHealth());
	//enemyのHPバーの更新
	enemyHpBar_->Update(enemy_->GetHealth(), enemy_->GetMaxHealth());

	//playerのエネルギーUIの更新
	energyInfoUI_->SetOverHeatState(player_->GetIsOverHeated());
	energyInfoUI_->Update(player_->GetEnergy(), player_->GetMaxEnergy());
	//bulletCounterUIの更新
	for (int i = 0; i < 4; i++) {
		bulletCounterUI_[i]->SetBulletCount(player_->GetWeapon(i)->GetBulletCount());
		bulletCounterUI_[i]->SetRemainingBulletCount(player_->GetWeapon(i)->GetRemainingBulletCount());
		bulletCounterUI_[i]->SetReloadingState(player_->GetWeapon(i)->GetIsReloading());
		bulletCounterUI_[i]->SetWeaponIconUV(static_cast<int>(player_->GetWeapon(i)->GetUnitPosition()));
		bulletCounterUI_[i]->Update();
	}

	//instructionSpriteの更新
	instructionSprite_->Update();

	if (player_->GetHealth() <= 0.0f) {
		//プレイヤーのHPが0以下になったらゲームオーバー
		behaviorRequest_ = SceneBehavior::GAMEOVER;

	} else if (enemy_->GetHealth() <= 0.0f) {
		//エネミーのHPが0以下になったらゲームクリア
		behaviorRequest_ = SceneBehavior::ENEMYDESTROYED;
	}

}

//====================================================================
// 敵撃破時の処理
//====================================================================

void GamePlayScene::InitializeEnemyDestroyed() {

	//スローモーションにする
	MyGame::RequestTimeScale(-1.0f, 1.0f,1.0f);
	//カメラをズームする
	CameraManager::GetInstance()->GetActiveCamera()->SetCameraStateRequest(Camera::GameCameraState::ENEMY_DESTROYED);
	//changeBehaviorTimerを初期化
	changeBehaviorTimer_.Initialize(2.0f, 0.0f);
}

void GamePlayScene::UpdateEnemyDestroyed() {

	//changeBehaviorTimerの更新
	changeBehaviorTimer_.Update();

	//changeBehaviorTimerが終了したらゲームクリアへ
	if (changeBehaviorTimer_.IsFinished()) {

		//ゲームクリアへ
		behaviorRequest_ = SceneBehavior::GAMECLEAR;

		//ズーム解除
		CameraManager::GetInstance()->GetActiveCamera()->SetCameraStateRequest(Camera::GameCameraState::FOLLOW);
	}
}


//====================================================================
// ゲームオーバー時の処理
//====================================================================
void GamePlayScene::InitializeGameOver() {

	fadeTimer_ = 2.0f;
	SceneManager::GetInstance()->ChangeScene("GAMEOVER", fadeTimer_);
}

void GamePlayScene::UpdateGameOver() {}
//====================================================================
// ゲームクリア時の処理
//====================================================================
void GamePlayScene::InitializeGameClear() {

	//スローモーション解除
	MyGame::RequestTimeScale(1.0f, 0.6f,0.0f);

	//フェード処理の開始
	fadeTimer_ = 4.0f;
	SceneManager::GetInstance()->ChangeScene("GAMECLEAR", fadeTimer_);
}

void GamePlayScene::UpdateGameClear() {

}

//====================================================================
// ポーズ時の処理
//====================================================================
void GamePlayScene::InitializePause() {}

void GamePlayScene::UpdatePause() {}

//====================================================================
//			全ての当たり判定のチェック
//====================================================================
void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance()->ClearGameCharacter();

	const std::vector<Bullet*>& bullets = bulletManager_->GetAllBullets();

	const std::vector<VerticalMissile*>& missiles = bulletManager_->GetAllMissiles();

	// プレイヤーの登録
	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(player_.get()));
	// 敵キャラクターの登録
	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(enemy_.get()));
	//BulletSensorの登録
	CollisionManager::GetInstance()->RegisterGameCharacter(static_cast<GameCharacter*>(enemy_->GetBulletSensor()));

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