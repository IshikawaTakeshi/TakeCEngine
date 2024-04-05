#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)

#include "WinApp.h"
#include "Logger.h"
#include "DirectXCommon.h"

//////////////////////////////////////////////////////////
//Windowsアプリでのエントリーポイント(main関数)
//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* winApp = WinApp::GetInstance();
	winApp->Init();

	DirectXCommon* directXCommon = DirectXCommon::GetInstance();
	directXCommon->InitializeDXGIDevice();

	//////////////////////////////////////////////////////////
	//メインループ
	//////////////////////////////////////////////////////////
	
	//ウィンドウの×ボタンが押されるまでループ
	while (winApp->ProcessMessage() == 0) {
		
	}

	//////////////////////////////////////////////////////////
	//出力ウィンドウへの文字出力
	//////////////////////////////////////////////////////////
	Logger::Log("Hello,DirectX!\n");

	return 0;
}



