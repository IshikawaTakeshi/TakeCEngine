#include "MyGame.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//leakCheckerの生成
	D3DResourceLeakChecker leakCheck;
	//ゲームクラスの生成
	std::unique_ptr<TakeC::TakeCFrameWork> game = std::make_unique<MyGame>();
	//ゲームの実行
	TakeC::ResourceRootConfig resources{
		.gameRoot = "Assets",
		.engineRoot = "EngineContent",
	};
	game->Run(L"A_CORE", resources);
	
	return 0; //終了
}
