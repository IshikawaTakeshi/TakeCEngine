#include "MyGame.h"

//////////////////////////////////////////////////////////
//Windowsアプリでのエントリーポイント(main関数)
//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//leakCheckerの生成
	D3DResourceLeakChecker* leakCheck = new D3DResourceLeakChecker;

	TakeCFrameWork* game = new MyGame();
	game->Run();

	delete leakCheck;
	
	return 0;
}



