#pragma once
#include "EngineIncludes.h"

#include "AbstractSceneFactory.h"

class TakeCFrameWork {
public:

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

	static ParticleManager* GetParticleManager();

protected:

	std::unique_ptr<WinApp> winApp_ = nullptr;
	std::unique_ptr<DirectXCommon> directXCommon_ = nullptr;
	std::unique_ptr<SrvManager> srvManager_ = nullptr;
	Input* input_ = nullptr;
	AudioManager* audio_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	Object3dCommon* object3dCommon_ = nullptr;
	ParticleCommon* particleCommon_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
	static std::unique_ptr<ParticleManager> particleManager_;
	
	//終了フラグ
	bool isEnd_ = false;
};

