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

#include "Quaternion.h"


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
	Quaternion q1 = { 2.0f,3.0f,4.0f,1.0f };
	Quaternion q2 = { 1.0f,3.0f,5.0f,2.0f };
	Quaternion identity = QuaternionMath::IdentityQuaternion();
	Quaternion conjugate = QuaternionMath::Conjugate(q1);
	float norm = QuaternionMath::Norm(q1);
	Quaternion normal = QuaternionMath::Normalize(q1);
	Quaternion inverse = QuaternionMath::Inverse(q1);
	Quaternion mul1 = QuaternionMath::Multiply(q1, q2);
	Quaternion mul2 = QuaternionMath::Multiply(q2, q1);
};