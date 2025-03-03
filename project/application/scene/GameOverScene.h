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

//===== Application =====//
#include "application/SkyBox/SkyBox.h"
#include "application/Ground/Ground.h"


class GameOverScene : public BaseScene {
public:
	enum Phase {
		FIRST,
		SECOND,
		FINAL,
	};
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
	AudioManager::SoundData gameOverBGM;
	bool isSoundPlay = false;
	std::unique_ptr<Camera> gameOverCamera_ = nullptr;
	// 天球
	std::unique_ptr<SkyBox> skybox_ = nullptr;
	// 地面
	std::unique_ptr<Ground> ground_ = nullptr;

	std::unique_ptr<Object3d> GameOverText_ = nullptr;

	float lerpTime_ = 0.0f;

	Phase phase_;

	bool changePhase_ = false;
};