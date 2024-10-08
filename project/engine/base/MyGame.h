#pragma once
#include "TakeCFrameWork.h"

class MyGame : public TakeCFrameWork {
public:

	MyGame() = default;
	~MyGame() = default;

	//初期化
	void Initialize() override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update()override;

	//描画処理
	void Draw()override;

private:

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

};

