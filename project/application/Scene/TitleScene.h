#pragma once
#include "BaseScene.h"

#include "audio/Audio.h"
#include "camera/Camera.h"
#include "camera/CameraManager.h"
#include "io/Input.h"
#include "base/ModelManager.h"
#include "2d/Sprite.h"
#include "2d/SpriteCommon.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "engine/SkyBox/SkyBox.h"

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

};

