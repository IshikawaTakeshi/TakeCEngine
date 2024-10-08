#pragma once
#include "EngineIncludes.h"

class TakeCFrameWork {
public:

	//デストラクタ
	virtual ~TakeCFrameWork() = default;
	//初期化
	virtual void Initialize();
	//終了処理
	virtual void Finalize();
	//更新処理
	virtual void Update();
	//描画処理
	virtual void Draw() = 0;
	//終了フラグの取得
	virtual bool IsEndRequest() const { return isEnd_; }

	//実行処理
	void Run();

protected:

	WinApp* winApp_ = nullptr;
	DirectXCommon* directXCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	Input* input_ = nullptr;
	AudioManager* audio_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	Object3dCommon* object3dCommon_ = nullptr;


	//終了フラグ
	bool isEnd_ = false;
};

