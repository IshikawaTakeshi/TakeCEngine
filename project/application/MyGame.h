#pragma once
#include "engine/EngineIncludes.h"

class MyGame {
public:

	MyGame() = default;
	~MyGame() = default;

	//初期化
	void Initialize();

	//終了処理
	void Finalize();

	//更新処理
	void Update();

	//描画処理
	void Draw();

	//終了フラグの取得
	bool IsEnd() const { return isEnd_; }

private:

	//==============   base   ==============//

	WinApp* winApp = nullptr;
	DirectXCommon* directXCommon = nullptr;
	SrvManager* srvManager = nullptr;
	Input* input = nullptr;
	std::shared_ptr<Audio> audio = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;
	ModelManager* modelManager = nullptr;
	TextureManager* textureManager = nullptr;

	//==============   game   ==============//


	//サウンドデータ
	Audio::SoundData soundData1;
	// カメラ
	std::shared_ptr<Camera> camera0 = nullptr;
	std::shared_ptr<Camera> camera1 = nullptr;
	//スプライト
	std::shared_ptr<Sprite> sprite = nullptr;
	//3Dオブジェクト
	std::shared_ptr <Object3d> object3d = nullptr;
	std::shared_ptr <Object3d> object3d1 = nullptr;

	//終了フラグ
	bool isEnd_ = false;
};

