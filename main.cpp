#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)

#include "WinApp.h"
#include "Logger.h"
#include "DirectXCommon.h"

#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

//////////////////////////////////////////////////////////
//Windowsアプリでのエントリーポイント(main関数)
//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* winApp = WinApp::GetInstance();
	winApp->Init();

	DirectXCommon* directXCommon = DirectXCommon::GetInstance();
	directXCommon->Initialize();

	//////////////////////////////////////////////////////////
	//メインループ
	//////////////////////////////////////////////////////////
	
	//ウィンドウの×ボタンが押されるまでループ
	while (winApp->ProcessMessage() == 0) {
		
		directXCommon->PreDraw();



		directXCommon->PostDraw();

	}

	IDXGIDebug1* debug;
	if(SUCCEEDED(DXGIGetDebugInterface1(0,IID_PPV_ARGS(&debug)))){
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	return 0;
}



