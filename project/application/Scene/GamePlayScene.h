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
#include "application/Scene/GamePlaySceneBehavior.h"
#include "application/Scene/SceneState/SceneStateManager.h"
// UI
#include "application/UI/ActionButtonICon.h"
#include "application/UI/PauseMenuUI.h"

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

	SceneStateManager& GetSceneStateManager() { return sceneStateManager_; }

	void SetPauseMenuActive(bool isActive) { isPauseMenuActive_ = isActive; }

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

	// 画面遷移時間
	float fadeTimer_ = 0.0f;
	// ポーズメニューがアクティブかどうか
	bool isPauseMenuActive_ = false; 
};