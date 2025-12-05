#pragma once
#include "LevelData.h"
#include "engine/audio/Audio.h"

//前方宣言
class SceneManager;

//============================================================================
// BaseScene class
//============================================================================
class BaseScene {
public:
	
	/// <summary>
	/// デストラクタ(仮想デストラクタ)
	/// </summary>
	virtual ~BaseScene() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize() = 0;

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	virtual void UpdateImGui() = 0;

	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;

	virtual void DrawSprite() = 0;

	/// <summary>
	/// シーンマネージャーのセット
	/// </summary>
	/// <param name="sceneManager"></param>
	virtual void SetSceneManager(SceneManager* sceneManager) {sceneManager_ = sceneManager; }

protected:

	//サウンドデータ
	AudioManager::SoundData BGM_;
	float bgmVolume_ = 0.05f;
	bool isSoundPlay_ = false;

	//シーンマネージャー(このクラスで解放しないこと)
	SceneManager* sceneManager_ = nullptr;
};