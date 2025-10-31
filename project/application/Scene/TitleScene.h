#pragma once
//base class
#include "application/Scene/BaseScene.h"

//engine
#include "engine/audio/Audio.h"
#include "engine/camera/Camera.h"
#include "engine/camera/CameraManager.h"
#include "engine/io/Input.h"
#include "engine/base/ModelManager.h"
#include "engine/2d/Sprite.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/SkyBox/SkyBox.h"

//application
#include "application/UI/PushStartUI.h"
#include "application/UI/PhaseMessageUI.h"

//============================================================================
// TitleScene class
//============================================================================
class TitleScene : public BaseScene {
public:

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

	//サウンドデータ
	//AudioManager::SoundData soundData1;
	// カメラ
	std::shared_ptr<Camera> camera0_ = nullptr;
	std::shared_ptr<Camera> camera1_ = nullptr;
	//SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;
	//スプライト
	std::unique_ptr<Sprite> titleTextSprite_ = nullptr;
	// 「PRESS START」スプライトUI
	std::unique_ptr<PushStartUI> pushStartUI_ = nullptr;

	// フェーズメッセージUI
	std::unique_ptr<PhaseMessageUI> phaseMessageUI_ = nullptr;

};