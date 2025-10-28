#pragma once
#include "BaseScene.h"

#include "Audio.h"
#include "Camera.h"
#include "CameraManager.h"
#include "Input.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "3d/Particle/Particle3d.h"
#include "3d/Particle/ParticleCommon.h"
#include "base/Particle/ParticleManager.h"
#include "3d/Particle/ParticleEmitter.h"
#include "SkyBox/SkyBox.h"

//============================================================================
// GameClearScene class
//============================================================================
class GameClearScene : public BaseScene {
public:

	//フェーズ
	enum Phase {
		FIRST,
		SECOND,
		FINAL,
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
	//ImGuiの更新
	void UpdateImGui() override;
	//描画処理
	void Draw() override;

private:

	//サウンドデータ
	AudioManager::SoundData gameClearBGM;
	//サウンド再生フラグ
	bool isSoundPlay = false;
	//カメラ
	std::unique_ptr<Camera> gameClearCamera_ = nullptr;

	// SkyBox
	std::unique_ptr<SkyBox> skybox_ = nullptr;

	//スプライト
	std::unique_ptr<Sprite> clearTextSprite_ = nullptr;
	//パーティクル
	float lerpTime_ = 0.0f;
	//フェーズ管理
	Phase phase_;
	//フェーズ変更フラグ
	bool changePhase_ = false;

};