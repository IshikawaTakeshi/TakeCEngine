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

	WinApp* winApp = nullptr;
	DirectXCommon* directXCommon = nullptr;
	SrvManager* srvManager = nullptr;
	Input* input = nullptr;
	std::shared_ptr<Audio> audio = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	Object3dCommon* object3dCommon = nullptr;


	//終了フラグ
	bool isEnd_ = false;
};

