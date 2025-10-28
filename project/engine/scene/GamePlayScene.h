#pragma once
#include "BaseScene.h"

#include "audio/Audio.h"
#include "io/Input.h"
#include "engine/camera/Camera.h"
#include "engine/camera/CameraManager.h"
#include "ModelManager.h"
#include "SkyBox/SkyBox.h"
#include "engine/2d/Sprite.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/3d/Particle/Particle3d.h"
#include "engine/3d/Particle/ParticleCommon.h"
#include "engine/3d/Particle/GPUParticle.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include "engine/base/Particle/ParticleManager.h"
#include "engine/Entity/LevelObject/LevelObject.h"
#include "engine/Utility/Timer.h"

//app
#include "application/Ground/Ground.h"
#include "application/Entity/Player/Player.h"
#include "application/Entity/Enemy/Enemy.h"
#include "application/UI/HPBar.h"
#include "application/UI/PlayerReticle.h"
#include "application/UI/EnergyInfoUI.h"
#include "application/UI/BulletCounterUI.h"

//=============================================================================
// GamePlayScene class
//=============================================================================
class GamePlayScene : public BaseScene {
public:

	//シーンの状態
	enum class SceneBehavior {
		GAMESTART, // ゲーム開始状態
		GAMEPLAY, // ゲームプレイ状態
		ENEMYDESTROYED, // 敵撃破状態
		GAMEOVER, // ゲームオーバー状態
		GAMECLEAR, // ゲームクリア状態
		PAUSE,  // ポーズ状態
	};

	//=========================================================================
	// functions
	//=========================================================================

	//初期化
	void Initialize() override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update() override;

	//ImGuiの更新処理
	void UpdateImGui() override;

	//描画処理
	void Draw() override;

private:

	//ゲーム開始時の初期化
	void InitializeGameStart();
	//ゲームプレイ時の初期化
	void InitializeGamePlay();
	void InitializeEnemyDestroyed();
	//ゲームオーバー時の初期化
	void InitializeGameOver();
	//ゲームクリア時の初期化
	void InitializeGameClear();
	//ポーズ時の初期化
	void InitializePause();

	//ゲーム開始時の更新
	void UpdateGameStart();
	//ゲームプレイ時の更新
	void UpdateGamePlay();
	void UpdateEnemyDestroyed();
	//ゲームオーバー時の更新
	void UpdateGameOver();
	//ゲームクリア時の更新
	void UpdateGameClear();
	//ポーズ時の更新
	void UpdatePause();

	//全ての当たり判定のチェック
	void CheckAllCollisions();

private:

	//状態遷移リクエスト
	std::optional<SceneBehavior> behaviorRequest_ = std::nullopt;
	//シーンの状態
	SceneBehavior behavior_ = SceneBehavior::GAMEPLAY;

	//サウンドデータ
	//AudioManager::SoundData BGM;
	bool isSoundPlay = false;
	// カメラ
	std::shared_ptr<Camera> gameCamera_ = nullptr;
	std::shared_ptr<Camera> debugCamera_ = nullptr;
	//SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	//player
	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<HPBar> playerHpBar_ = nullptr;
	std::unique_ptr<PlayerReticle> playerReticle_ = nullptr;
	std::unique_ptr<EnergyInfoUI> energyInfoUI_ = nullptr;
	std::vector<std::unique_ptr<BulletCounterUI>> bulletCounterUI_;
	//enemy
	std::unique_ptr<Enemy> enemy_ = nullptr;
	std::unique_ptr<HPBar> enemyHpBar_ = nullptr;

	std::unique_ptr<BulletManager> bulletManager_ = nullptr;

	std::map<std::string,std::unique_ptr<LevelObject>> levelObjects_;

	std::unique_ptr<Sprite> instructionSprite_ = nullptr;

	//画面遷移時間
	float fadeTimer_ = 0.0f;
	//状態遷移用タイマー
	Timer changeBehaviorTimer_;
};