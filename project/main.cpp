#include "MyGame.h"

//////////////////////////////////////////////////////////
//Windowsアプリでのエントリーポイント(main関数)
//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//leakCheckerの生成
	D3DResourceLeakChecker* leakCheck = new D3DResourceLeakChecker;

	//ゲームクラスの生成と初期化
	MyGame myGame;
	myGame.Initialize();

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		if (myGame.IsEnd() == true) {
			break;
		}
		myGame.Update();
		myGame.Draw();
	}

	myGame.Finalize();

	delete leakCheck;
	
	return 0;
}



