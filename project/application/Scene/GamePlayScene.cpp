#include "GamePlayScene.h"
#include "Collision/CollisionManager.h"
#include "ImGuiManager.h"
#include "MyGame.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "Vector3Math.h"
#include <format>
#include <numbers>

// SceneState includes
#include "application/Scene/SceneState/GamePlayScene/SceneStateEnemyDestroyed.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameClear.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameOver.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGamePlay.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStateGameStart.h"
#include "application/Scene/SceneState/GamePlayScene/SceneStatePause.h"

// UI
#include "application/UI/BreakGaugeUI.h"

//====================================================================
//			初期化
//====================================================================

void GamePlayScene::Initialize() {

	// BGM読み込み
	BGM_ = AudioManager::GetInstance().LoadSound("GamePlaySceneBGM.mp3");
	bgmVolume_ = 0.2f;

	// Camera0
	gameCamera_ = std::make_shared<Camera>();
	gameCamera_->Initialize(
		TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),
		"CameraConfig_GameScene.json");
	gameCamera_->RequestCameraState(Camera::GameCameraState::FOLLOW);
	TakeC::CameraManager::GetInstance().AddCamera("gameCamera", *gameCamera_);

	// Camera1
	debugCamera_ = std::make_shared<Camera>();
	debugCamera_->Initialize(
		TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),
		"CameraConfig_GameScene.json");
	TakeC::CameraManager::GetInstance().AddCamera("debugCamera", *debugCamera_);

	// lightCamera
	lightCamera_ = std::make_shared<Camera>();
	lightCamera_->Initialize(
		TakeC::CameraManager::GetInstance().GetDirectXCommon()->GetDevice(),
		"CameraConfig_LightCamera.json");
	lightCamera_->SetProjectionChanged(true);
	TakeC::CameraManager::GetInstance().AddCamera("lightCamera", *lightCamera_);

	// デフォルトカメラの設定
	Object3dCommon::GetInstance().SetDefaultCamera(
		TakeC::CameraManager::GetInstance().GetActiveCamera());
	ParticleCommon::GetInstance().SetDefaultCamera(
		TakeC::CameraManager::GetInstance().GetActiveCamera());

	// levelObjectの初期化
	sceneManager_->LoadLevelData("levelData_gameScene_5");
	levelObjects_ = std::move(sceneManager_->GetLevelObjects());

	for (auto& object : levelObjects_) {
		object.second->SetCamera(Object3dCommon::GetInstance().GetDefaultCamera());
	}

	// SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance().GetDirectXCommon(),
		"skyBox_blueSky.dds");
	skyBox_->SetMaterialColor({ 0.2f, 0.2f, 0.2f, 1.0f });

	// BulletManager
	bulletManager_ = std::make_unique<BulletManager>();
	bulletManager_->Initialize(&Object3dCommon::GetInstance(), 50); // 弾の最大数

	// playerInputProvider
	player_ = std::make_unique<Player>();
	inputProvider_Player = std::make_unique<PlayerInputProvider>(player_.get());

	// player
	player_->SetInputProvider(inputProvider_Player.get());
	player_->Initialize(&Object3dCommon::GetInstance(),
		"Player_Model_Ver2.0.gltf");
	player_->WeaponInitialize(&Object3dCommon::GetInstance(),
		bulletManager_.get());
	player_->GetObject3d()->SetAnimation(
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Running"));
	player_->SetTranslate({ 0.0f, 0.0f, 0.0f });
	// Enemy
	enemy_ = std::make_unique<Enemy>();
	inputProvider_Enemy = std::make_unique<EnemyInputProvider>(enemy_.get());
	enemy_->SetInputProvider(inputProvider_Enemy.get());
	enemy_->Initialize(&Object3dCommon::GetInstance(), "Enemy_Model.gltf");
	enemy_->WeaponInitialize(&Object3dCommon::GetInstance(),
		bulletManager_.get());
	enemy_->GetObject3d()->SetAnimation(
		TakeCFrameWork::GetAnimationManager()->FindAnimation(
			"Player_Model_Ver2.0.gltf", "Running"));

	// playerHpBar
	playerHpBar_ = std::make_unique<HPBar>();
	playerHpBar_->Initialize(&SpriteCommon::GetInstance(), "PlayerHPName.json",
		"black.png", "flontHp.png");
	playerHpBar_->SetSize({ 200.0f, 10.0f });     // HPバーのサイズ
	playerHpBar_->SetPosition({ 50.0f, 500.0f }); // HPバーの位置
	// enemyHpBar
	enemyHpBar_ = std::make_unique<HPBar>();
	enemyHpBar_->Initialize(&SpriteCommon::GetInstance(), "EnemyHPName.json",
		"black.png", "flontHp.png");
	enemyHpBar_->SetSize({ 400.0f, 10.0f });     // HPバーのサイズ
	enemyHpBar_->SetPosition({ 300.0f, 35.0f }); // HPバーの位置
	// playerReticle
	playerReticle_ = std::make_unique<PlayerReticle>();
	playerReticle_->Initialize();
	// energyInfoUI
	energyInfoUI_ = std::make_unique<EnergyInfoUI>();
	energyInfoUI_->Initialize(&SpriteCommon::GetInstance(), "black.png",
		"flontHp.png");
	energyInfoUI_->SetSize({ 400.0f, 10.0f });      // エネルギーUIのサイズ
	energyInfoUI_->SetPosition({ 300.0f, 525.0f }); // エネルギーUIの位置
	// bulletCounterUI
	bulletCounterUI_.resize(4); // 4つの弾数カウンターを用意
	bulletCounterUI_[0] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[0]->Initialize(TakeCFrameWork::GetSpriteManager(),{ 760.0f, 470.0f });
	bulletCounterUI_[1] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[1]->Initialize(TakeCFrameWork::GetSpriteManager(),{ 900.0f, 470.0f });
	bulletCounterUI_[2] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[2]->Initialize(TakeCFrameWork::GetSpriteManager(),{ 760.0f, 540.0f });
	bulletCounterUI_[3] = std::make_unique<BulletCounterUI>();
	bulletCounterUI_[3]->Initialize(TakeCFrameWork::GetSpriteManager(),{ 900.0f, 540.0f });

	// フェーズメッセージUI
	phaseMessageUI_ = std::make_unique<PhaseMessageUI>();
	phaseMessageUI_->Initialize();

	// 操作説明UI
	for (size_t i = 0; i < 7; i++) {

		// アクションボタンアイコンUI
		TakeCFrameWork::GetUIManager()->CreateUI<ActionButtonICon>(
			"InstructionIcon" + std::to_string(i) + ".json",
			inputProvider_Player.get(), static_cast<CharacterActionInput>(i + 3));
	}
	// 警告表示UI
	TakeCFrameWork::GetUIManager()->CreateUI<WarningUI>("WarningUI");

	// BreakGaugeUI
	TakeCFrameWork::GetUIManager()->CreateUI<BreakGaugeUI>("BreakGauge_Player",
		"Player");
	TakeCFrameWork::GetUIManager()->CreateUI<BreakGaugeUI>("BreakGauge_Enemy",
		"Enemy");

	// アクションアイコンUI
	actionIconSprites_.resize(3);

	for (size_t i = 0; i < actionIconSprites_.size(); i++) {
		actionIconSprites_[i] = TakeCFrameWork::GetSpriteManager()->CreateFromJson(
			"ActionIcon" + std::to_string(i) + ".json");
	}

	// シーンステートの初期化
	sceneStateManager_.RegisterState(SceneState::GAMESTART,
		std::make_unique<SceneStateGameStart>());
	sceneStateManager_.RegisterState(SceneState::GAMEPLAY,
		std::make_unique<SceneStateGamePlay>());
	sceneStateManager_.RegisterState(
		SceneState::ENEMYDESTROYED, std::make_unique<SceneStateEnemyDestroyed>());
	sceneStateManager_.RegisterState(SceneState::GAMEOVER,
		std::make_unique<SceneStateGameOver>());
	sceneStateManager_.RegisterState(SceneState::GAMECLEAR,
		std::make_unique<SceneStateGameClear>());
	sceneStateManager_.RegisterState(SceneState::PAUSE,
		std::make_unique<SceneStatePause>());
	sceneStateManager_.Initialize(SceneState::GAMESTART, this);

	// ShadowMapEffectを有効化
	TakeCFrameWork::GetPostEffectManager()->SetEffectActive("ShadowMapEffect",
		true);
}

//====================================================================
//			終了処理
//====================================================================

void GamePlayScene::Finalize() {

	AudioManager::GetInstance().SoundUnload(&BGM_);         // BGMの解放
	CollisionManager::GetInstance().ClearGameCharacter();   // 当たり判定の解放
	TakeC::CameraManager::GetInstance().ResetCameras();     // カメラのリセット
	TakeCFrameWork::GetParticleManager()->ClearParticles(); // パーティクルの解放
	TakeCFrameWork::GetParticleManager()->ClearEmitters();  // エミッターの解放
	TakeCFrameWork::GetLightManager()
		->ClearAllPointLights();                 // ポイントライトの解放
	TakeCFrameWork::GetSpriteManager()->Clear(); // スプライトの解放
	TakeCFrameWork::GetUIManager()->Clear();     // UIの解放
	bulletManager_->Finalize();                  // 弾マネージャーの解放
	player_.reset();
	skyBox_.reset();
}

//====================================================================
//			更新処理
//====================================================================
void GamePlayScene::Update() {

	// BGM再生
	if (!isSoundPlay_) {
		AudioManager::GetInstance().SoundPlayWave(BGM_, bgmVolume_, true);
		isSoundPlay_ = true;
	}

	// カメラの更新
	TakeC::CameraManager::GetInstance().Update();
	// SkyBoxの更新
	skyBox_->Update();

	// enemy
	enemy_->SetFocusTargetPos(player_->GetBodyPosition());
	enemy_->SetFocusTargetVelocity(player_->GetVelocity());

	// player
	player_->SetFocusTargetPos(enemy_->GetBodyPosition());
	player_->SetFocusTargetVelocity(enemy_->GetVelocity());

	// シーンステートの更新
	sceneStateManager_.Update(this);

	enemy_->Update();
	player_->Update();

	// 弾の更新
	bulletManager_->Update();

	for (auto& object : levelObjects_) {
		object.second->Update();
	}

	// フェーズメッセージUIの更新
	phaseMessageUI_->Update();

	// particleManager更新
	TakeCFrameWork::GetParticleManager()->Update();
	// LightManager更新
	Camera* lightCam =
		TakeC::CameraManager::GetInstance().FindCameraByName("lightCamera");
	lightCam->Update();
	TakeCFrameWork::GetLightManager()->UpdateShadowMatrix(
		lightCam, player_->GetObject3d()->GetWorldPosition());

	// 当たり判定の更新
	CheckAllCollisions();
}

void GamePlayScene::UpdateImGui() {

	TakeC::CameraManager::GetInstance().UpdateImGui();
	Object3dCommon::GetInstance().UpdateImGui();

	player_->UpdateImGui();
	enemy_->UpdateImGui();
	playerHpBar_->UpdateImGui("player");
	enemyHpBar_->UpdateImGui("enemy");
	playerReticle_->UpdateImGui();
	energyInfoUI_->UpdateImGui("player");
	phaseMessageUI_->UpdateImGui();
	for (int i = 0; i < 4; i++) {
		bulletCounterUI_[i]->UpdateImGui(std::format("bulletCounter{}", i));
	}

	// actionIconSprite
	for (size_t i = 0; i < actionIconSprites_.size(); i++) {
		actionIconSprites_[i]->UpdateImGui(std::format("actionIcon{}", i));
	}

	TakeCFrameWork::GetSpriteManager()->UpdateImGui("DefaultSprite");
}

//====================================================================
//			描画処理
//====================================================================

void GamePlayScene::Draw() {

	skyBox_->Draw(); // 天球の描画

#pragma region 背景スプライト描画
	// スプライトの描画前処理
	SpriteCommon::GetInstance().PreDraw();

#pragma endregion

#pragma region Object3d描画

	// Object3dの描画前Dispatch処理
	Object3dCommon::GetInstance().Dispatch();
	player_->GetObject3d()->Dispatch();
	enemy_->GetObject3d()->Dispatch();

	// for (auto& object : levelObjects_) {
	//	object->DisPatch();
	// }

	// Object3dの描画前処理
	Object3dCommon::GetInstance().PreDraw();
	// プレイヤーの描画
	player_->Draw();
	// 敵の描画
	enemy_->Draw();
	// 弾の描画
	bulletManager_->Draw();
	for (auto& object : levelObjects_) {
		object.second->Draw();
	}
	// Object3dCommon::GetInstance().PreDrawAddBlend();

#pragma endregion

  // 当たり判定の描画前処理
	enemy_->DrawCollider();
	player_->DrawCollider();
	// spotLightの描画
	TakeCFrameWork::GetLightManager()->DrawSpotLights();
	// 登録されたワイヤーフレームをすべて描画させる
	TakeCFrameWork::GetWireFrame()->Draw();

	// パーティクルの描画
	TakeCFrameWork::GetParticleManager()->Draw();
}

void GamePlayScene::DrawSprite() {

	// スプライトの描画前処理
	SpriteCommon::GetInstance().PreDraw();

	if (sceneStateManager_.GetCurrentStateType() != SceneState::ENEMYDESTROYED) {
		// プレイヤーのレティクルの描画
		playerReticle_->Draw();
		// HPバーの描画
		playerHpBar_->Draw(); // プレイヤーのHPバーの描画
		enemyHpBar_->Draw();  // 敵のHPバーの描画
		// エネルギーUIの描画
		energyInfoUI_->Draw();
		// フェーズメッセージUIの描画
		phaseMessageUI_->Draw();

		TakeCFrameWork::GetSpriteManager()->Draw();
	}
}

//====================================================================
//			影描画処理
//====================================================================
void GamePlayScene::DrawShadow() {

	// ライトカメラのセット
	const LightCameraInfo& lightCameraInfo =
		TakeCFrameWork::GetLightManager()->GetLightCameraInfo();
	// Object3dの影描画前処理
	Object3dCommon::GetInstance().PreDrawShadowPass();
	// 影の描画
	player_->DrawShadow(lightCameraInfo);
	enemy_->DrawShadow(lightCameraInfo);
	for (auto& object : levelObjects_) {
		object.second->DrawShadow(lightCameraInfo);
	}
}

//====================================================================
//			全ての当たり判定のチェック
//====================================================================
void GamePlayScene::CheckAllCollisions() {

	CollisionManager::GetInstance().ClearGameCharacter();

	const std::vector<Bullet*>& playerBullets =
		bulletManager_->GetAllPlayerBullets();
	const std::vector<Bullet*>& enemyBullets =
		bulletManager_->GetAllEnemyBullets();
	const std::vector<Bullet*>& playerBazookaBullets =
		bulletManager_->GetAllPlayerBazookaBullets();
	const std::vector<Bullet*>& enemyBazookaBullets =
		bulletManager_->GetAllEnemyBazookaBullets();
	const std::vector<VerticalMissile*>& playerMissiles =
		bulletManager_->GetAllPlayerMissiles();
	const std::vector<VerticalMissile*>& enemyMissiles =
		bulletManager_->GetAllEnemyMissiles();

	// プレイヤーの登録
	CollisionManager::GetInstance().RegisterGameCharacter(
		static_cast<GameCharacter*>(player_.get()));
	// 敵キャラクターの登録
	CollisionManager::GetInstance().RegisterGameCharacter(
		static_cast<GameCharacter*>(enemy_.get()));
	// BulletSensorの登録
	CollisionManager::GetInstance().RegisterGameCharacter(
		static_cast<GameCharacter*>(enemy_->GetBulletSensor()));

	// 弾の登録
	for (const auto& bullet : playerBullets) {
		if (bullet->IsActive()) {
			CollisionManager::GetInstance().RegisterGameCharacter(
				static_cast<GameCharacter*>(bullet));
		}
	}
	for (const auto& bullet : enemyBullets) {
		if (bullet->IsActive()) {
			CollisionManager::GetInstance().RegisterGameCharacter(
				static_cast<GameCharacter*>(bullet));
		}
	}

	// バズーカ弾の登録
	for (const auto& bazookaBullet : playerBazookaBullets) {
		if (bazookaBullet->IsActive()) {
			CollisionManager::GetInstance().RegisterGameCharacter(
				static_cast<GameCharacter*>(bazookaBullet));
		}
	}
	for (const auto& bazookaBullet : enemyBazookaBullets) {
		if (bazookaBullet->IsActive()) {
			CollisionManager::GetInstance().RegisterGameCharacter(
				static_cast<GameCharacter*>(bazookaBullet));
		}
	}
	// 垂直ミサイルの登録
	for (const auto& missile : playerMissiles) {
		if (missile->IsActive()) {
			CollisionManager::GetInstance().RegisterGameCharacter(
				static_cast<GameCharacter*>(missile));
		}
	}
	for (const auto& missile : enemyMissiles) {
		if (missile->IsActive()) {
			CollisionManager::GetInstance().RegisterGameCharacter(
				static_cast<GameCharacter*>(missile));
		}
	}

	// レベルオブジェクトの登録
	for (const auto& object : levelObjects_) {
		CollisionManager::GetInstance().RegisterGameCharacter(
			static_cast<GameCharacter*>(object.second.get()));
	}

	CollisionManager::GetInstance().CheckAllCollisionsForGameCharacter();
}