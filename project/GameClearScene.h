#pragma once
#include "scene/BaseScene.h"
#include "Camera.h"
#include "CameraManager.h"
#include "Input.h"
#include <memory>

class GameClearScene : public BaseScene {

public:

	//初期化
	void Initialize() override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update() override;

	//描画処理
	void Draw() override;

private:

};

