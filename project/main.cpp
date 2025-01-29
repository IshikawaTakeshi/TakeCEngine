#include "MyGame.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//leakCheckerの生成
	D3DResourceLeakChecker* leakCheck = new D3DResourceLeakChecker;
	//ゲームクラスの生成
	std::unique_ptr<TakeCFrameWork> game = std::make_unique<MyGame>();
	//ゲームの実行
	game->Run(L"CG4");
	//leakCheckerの解放
	delete leakCheck;
	
	return 0; //終了
}