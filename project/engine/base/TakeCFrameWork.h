#pragma once
#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "base/DirectXCommon.h"
#include "base/D3DResourceLeakChecker.h"
#include "base/TextureManager.h"
#include "base/ModelManager.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/WinApp.h"
#include "base/Particle/ParticleManager.h"
#include "3d/Object3dCommon.h"
#include "3d/Primitive/PrimitiveDrawer.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Light/LightManager.h"
#include "3d/Particle/ParticleEditor.h"
#include "2d/SpriteCommon.h"
#include "2d/WireFrame.h"
#include "Animation/Animator.h"
#include "audio/Audio.h"
#include "io/Input.h"
#include "camera/CameraManager.h"
#include "primitive/Sphere.h"
#include "PostEffect/PostEffectManager.h"
#include "PostEffect/RenderTexture.h"
#include "scene/SceneManager.h"
#include "scene/SceneTransition.h"
#include "scene/AbstractSceneFactory.h"
#include "Utility/Logger.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/JsonLoader.h"
#include "Utility/Timer.h"
#include "engine/math/Easing.h"

//DirectXTex
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
//imgui
#include "ImGuiManager.h"

#include <chrono>

//============================================================================
// TakeCFrameWork class
//============================================================================

class TakeCFrameWork {
public:

	//=========================================================================
	// functions
	//=========================================================================

	//デストラクタ
	virtual ~TakeCFrameWork() = default;
	//初期化
	virtual void Initialize(const std::wstring& titleName);
	//終了処理
	virtual void Finalize();
	//更新処理
	virtual void Update();
	//描画処理
	virtual void Draw() = 0;
	//終了フラグの取得
	virtual bool IsEndRequest() const { return isEnd_; }

	//実行処理
	void Run(const std::wstring& titleName);

	//=========================================================================
	// accessor
	//=========================================================================

	//ParticleManagerの取得
	static ParticleManager* GetParticleManager();
	//PostEffectManagerの取得
	static Animator* GetAnimator();
	//JsonLoaderの取得
	static JsonLoader* GetJsonLoader();
	//PrimitiveDrawerの取得
	static PrimitiveDrawer* GetPrimitiveDrawer();
	//PostEffectManagerの取得
	static PostEffectManager* GetPostEffectManager();
	//WireFrameの取得
	static WireFrame* GetWireFrame();
	//ゲーム起動時間の取得
	static float GetGameTime();
	//経過時間の取得
	static float GetDeltaTime() { return kDeltaTime; }

protected:

	//=========================================================================
	// variables
	//=========================================================================

	//windowsアプリケーション
	std::unique_ptr<WinApp> winApp_ = nullptr;
	//DirectX共通部分
	std::unique_ptr<DirectXCommon> directXCommon_ = nullptr;
	//srvマネージャー
	std::unique_ptr<SrvManager> srvManager_ = nullptr;
	//描画先テクスチャ
	std::unique_ptr<RenderTexture> renderTexture_ = nullptr;
	//入力管理クラス
	Input* input_ = nullptr;
	//オーディオ管理クラス
	AudioManager* audio_ = nullptr;
	//スプライト共通クラス
	SpriteCommon* spriteCommon_ = nullptr;
	//3Dオブジェクト共通クラス
	Object3dCommon* object3dCommon_ = nullptr;
	//パーティクル共通クラス
	ParticleCommon* particleCommon_ = nullptr;
	//シーンマネージャー
	SceneManager* sceneManager_ = nullptr;
	//シーン遷移管理クラス
	SceneTransition* sceneTransition_ = nullptr;
	//ImGui管理クラス
	ImGuiManager* imguiManager_ = nullptr;

	//シーンファクトリー
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;

	//アニメーション管理クラス
	static std::unique_ptr<Animator> animator_;
	//Jsonローダー
	static std::unique_ptr<JsonLoader> jsonLoader_;
	//パーティクルマネージャー
	static std::unique_ptr<ParticleManager> particleManager_;
	//プリミティブ描画クラス
	static std::unique_ptr<PrimitiveDrawer> primitiveDrawer_;
	//ポストエフェクトマネージャー
	static std::unique_ptr<PostEffectManager> postEffectManager_;
	//ワイヤーフレーム描画クラス
	static std::unique_ptr<WireFrame> wireFrame_;
	//LightManager
	static std::unique_ptr<LightManager> lightManager_;
	
	// ゲームの起動時間
	static std::chrono::steady_clock::time_point gameTime_;
	// ゲームの経過時間
	static float kDeltaTime;
	//終了フラグ
	bool isEnd_ = false;
	//時間倍率
	float timeScale_ = 1.0f;
};