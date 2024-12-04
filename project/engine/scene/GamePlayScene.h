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
#include "Particle3d.h"
#include "ParticleCommon.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"


class GamePlayScene : public BaseScene {
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

	//サウンドデータ
	AudioManager::SoundData soundData1;
	// カメラ
	std::shared_ptr<Camera> camera0_ = nullptr;
	std::shared_ptr<Camera> camera1_ = nullptr;
	//スプライト
	std::shared_ptr<Sprite> sprite_ = nullptr;
	//3Dオブジェクト
	std::shared_ptr <Object3d> object3d = nullptr;
	std::shared_ptr <Object3d> object3d1 = nullptr;
	//パーティクル発生器
	std::unique_ptr<ParticleEmitter> particleEmitter1_ = nullptr;
	std::unique_ptr<ParticleEmitter> particleEmitter2_ = nullptr;
	

	//MT4
	Vector3 axis = {};
	float angle = 0.44f;
	Matrix4x4 rotateMatrix = {};

};