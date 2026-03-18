#pragma once
#include "BaseScene.h"

#include "Input/Input.h"
#include "ModelManager.h"
#include "SkyBox/SkyBox.h"
#include "audio/Audio.h"
#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Particle/GPUParticle.h"
#include "engine/3d/Particle/Particle3d.h"
#include "engine/3d/Particle/ParticleCommon.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include "engine/Entity/LevelObject/LevelObject.h"
#include "engine/Utility/Timer.h"
#include "engine/base/Particle/ParticleManager.h"
#include "engine/camera/Camera.h"
#include "engine/camera/CameraManager.h"

// app
#include "application/Entity/Enemy/Enemy.h"
#include "application/Entity/Player/Player.h"
#include "application/Entity/Behavior/BehaviorTreeEditor.h"
#include "application/Ground/Ground.h"
#include "application/Scene/GamePlaySceneBehavior.h"
#include "application/Scene/SceneState/SceneStateManager.h"
#include "application/UI/ActionButtonICon.h"
#include "application/UI/BulletCounterGaugeUI.h"
#include "application/UI/BulletCounterUI.h"
#include "application/UI/EnergyInfoUI.h"
#include "application/UI/HPBar.h"
#include "application/UI/PhaseMessageUI.h"
#include "application/UI/PlayerReticle.h"
#include "application/UI/WarningUI.h"

//=============================================================================
// GamePlayScene class
//=============================================================================
class GamePlayScene : public BaseScene {
public:
	//=========================================================================
	// functions
	//=========================================================================

	// 初期化
	void Initialize() override;

	// 終了処理
	void Finalize() override;

	// 更新処理
	void Update() override;

	// ImGuiの更新処理
	void UpdateImGui() override;

	// 描画処理
	void Draw() override;
	// スプライト描画処理
	void DrawSprite() override;
	// シャドウ描画処理
	void DrawShadow() override;

	//=========================================================================
	// getter（ステートクラスからのアクセス用）
	//=========================================================================

	Player* GetPlayer() const { return player_.get(); }
	Enemy* GetEnemy() const { return enemy_.get(); }
	HPBar* GetPlayerHpBar() const { return playerHpBar_.get(); }
	HPBar* GetEnemyHpBar() const { return enemyHpBar_.get(); }
	PlayerReticle* GetPlayerReticle() const { return playerReticle_.get(); }
	EnergyInfoUI* GetEnergyInfoUI() const { return energyInfoUI_.get(); }
	BulletCounterUI* GetBulletCounterUI(int index) const {
		return bulletCounterUI_[index].get();
	}
	BulletCounterGaugeUI* GetBulletCounterGaugeUI() const {
		return bulletCounterGaugeUI_.get();
	}
	PhaseMessageUI* GetPhaseMessageUI() const { return phaseMessageUI_.get(); }
	SceneStateManager& GetSceneStateManager() { return sceneStateManager_; }

private:
	// 全ての当たり判定のチェック
	void CheckAllCollisions();

private:
	// ステートマネージャー
	SceneStateManager sceneStateManager_;

	// カメラ
	std::unique_ptr<Camera> gameCamera_ = nullptr;
	std::unique_ptr<Camera> debugCamera_ = nullptr;
	std::unique_ptr<Camera> lightCamera_ = nullptr;
	// SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	// player
	std::unique_ptr<Player> player_ = nullptr;
	// プレイヤー入力プロバイダ
	std::unique_ptr<PlayerInputProvider> inputProvider_Player = nullptr;
	std::unique_ptr<HPBar> playerHpBar_ = nullptr;
	std::unique_ptr<PlayerReticle> playerReticle_ = nullptr;
	std::unique_ptr<EnergyInfoUI> energyInfoUI_ = nullptr;
	std::vector<std::unique_ptr<BulletCounterUI>> bulletCounterUI_;
	std::unique_ptr<BulletCounterGaugeUI> bulletCounterGaugeUI_;
	// enemy
	std::unique_ptr<EnemyInputProvider> inputProvider_Enemy = nullptr;
	std::unique_ptr<Enemy> enemy_ = nullptr;
	std::unique_ptr<HPBar> enemyHpBar_ = nullptr;
	std::unique_ptr<BehaviorTreeEditor> behaviorTreeEditor_ = nullptr;

	std::unique_ptr<BulletManager> bulletManager_ = nullptr;

	std::map<std::string, std::unique_ptr<LevelObject>> levelObjects_;

	//	操作説明スプライト
	std::vector<std::unique_ptr<Sprite>> instructionSprites_;
	// 　アクションアイコン
	std::vector<Sprite*> actionIconSprites_;
	// フェーズメッセージUI
	std::unique_ptr<PhaseMessageUI> phaseMessageUI_ = nullptr;

	// 画面遷移時間
	float fadeTimer_ = 0.0f;
};