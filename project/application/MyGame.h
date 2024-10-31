#pragma once
#include "TakeCFrameWork.h"

class MyGame : public TakeCFrameWork {
public:

	MyGame() = default;
	~MyGame() = default;

	//初期化
	void Initialize(const std::wstring& titleName) override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update()override;

	//描画処理
	void Draw()override;

private:

	std::shared_ptr<BaseScene> currentScene_;
};