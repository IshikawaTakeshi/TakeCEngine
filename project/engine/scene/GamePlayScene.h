#pragma once
#include "Audio.h"
#include "Camera.h"
#include "CameraManager.h"
#include "Input.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"

class GamePlayScene {
public:

	GamePlayScene() = default;
	~GamePlayScene() = default;

	//初期化
	void Initialize();

	//終了処理
	void Finalize();

	//更新処理
	void Update();

	//描画処理
	void Draw();

private:

	//サウンドデータ
	AudioManager::SoundData soundData1;
	// カメラ
	std::shared_ptr<Camera> camera0 = nullptr;
	std::shared_ptr<Camera> camera1 = nullptr;
	//スプライト
	std::shared_ptr<Sprite> sprite = nullptr;
	//3Dオブジェクト
	std::shared_ptr <Object3d> object3d = nullptr;
	std::shared_ptr <Object3d> object3d1 = nullptr;
};

