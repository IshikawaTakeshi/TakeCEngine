#include "WinApp.h"
#include "engine/Input/Input.h"
#include <cassert>

#pragma region imgui
#if defined(_DEBUG) || defined(_DEVELOP)
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG

//静的メンバ変数の初期化
float TakeC::WinApp::widthPercent_ = float(WinApp::kScreenWidth) / WinApp::kDebugScreenWidth_;
float TakeC::WinApp::heightPercent_ = float(WinApp::kScreenHeight) / WinApp::kDebugScreenHeight_;

//=====================================================================
//			初期化
//=====================================================================

void TakeC::WinApp::Initialize(const wchar_t title[]) {

	HRESULT hr;
	hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	// ウィンドウプロシージャ
	wc_.lpfnWndProc = WinApp::WindowProc;
	//ウィンドウクラス名()
	wc_.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc_);
	//ゲームウィンドウの作成
	CreateGameWindow(title);

	assert(SUCCEEDED(hr));

#if defined(_DEBUG) || defined(_DEVELOP)

	//debugController
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		//デバッグレイヤーを有効化する
		debugController_->EnableDebugLayer();
		//さらにGPU側にもチェックを行うようにする
		debugController_->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG

}

//=====================================================================
//			終了・開放処理
//=====================================================================

void TakeC::WinApp::Finalize() {

	CloseWindow(hwnd_);
	CoUninitialize();
}

//=======================================================================
//			メッセージの処理
//=======================================================================

bool TakeC::WinApp::ProcessMessage() {

	MSG msg{};//メッセージ

	//Windowにメッセージが来てたら最優先で処理させる
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 終了メッセージが来たらループを抜ける
	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}

//=======================================================================
//ウィンドウプロシージャ
//=======================================================================

LRESULT CALLBACK TakeC::WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
#if defined(_DEBUG) || defined(_DEVELOP)
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}
#endif // _DEBUG


	//メッセージに応じてゲーム特有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;

		// F11 でフルスクリーン切替
	case WM_KEYDOWN:
		if (TakeC::Input::GetInstance().PushKey(DIK_F11)) {
			// hwnd を使ってインスタンスのフラグを取得するため、ウィンドウユーザーデータにポインタを設定しておく必要があります。
			// ここでは SetWindowLongPtr / GetWindowLongPtr を使って保存されているポインタを取得する想定です。
			LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (ptr) {
				WinApp* app = reinterpret_cast<WinApp*>(ptr);
				app->ToggleFullscreen();
			}
		}
		break;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//=======================================================================
//			ゲームウィンドウの作成
//=======================================================================

void TakeC::WinApp::CreateGameWindow(const wchar_t title[]) {

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc_ = { 0,0,kWindowWidth,kWindowHeight };

	//クライアント領域を元に実際のサイズにwrc_を変更してもらう
	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(
		wc_.lpszClassName,       //利用するクラス名
		title,                 //タイトルバーの文字(何でもいい,引数に入れることもできる)
		WS_OVERLAPPEDWINDOW,    //よく見るウィンドウスタイル
		CW_USEDEFAULT,          //表示X座標(Windowsに任せる)
		CW_USEDEFAULT,          //表示Y座標(WindowsOSに任せる)
		wrc_.right - wrc_.left,   //ウィンドウ横幅
		wrc_.bottom - wrc_.top,   //ウィンドウ縦幅
		nullptr,                //親ウィンドウハンドル
		nullptr,                //メニューハンドル
		wc_.hInstance,           //インスタンスハンドル
		nullptr                 //オプション
	);

	// ウィンドウインスタンスを WindowLongPtr に保存（WindowProc で参照するため）
	SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

#if defined(_DEBUG) || defined(_DEVELOP)

#else

	SetFullscreen(true); // 起動時はフルスクリーンにする

#endif // DEBUG


	//ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
}

//=======================================================================
//            フルスクリーン切替の実装
//=======================================================================

void TakeC::WinApp::SetFullscreen(bool enable) {
	if (enable == isFullscreen_) {
		return; // 既にその状態
	}

	if (enable) {
		// ウィンドウモード -> フルスクリーン（ボーダレス）
		// 現在のウィンドウスタイル/位置を保存
		prevStyle_ = GetWindowLongPtr(hwnd_, GWL_STYLE);
		prevExStyle_ = GetWindowLongPtr(hwnd_, GWL_EXSTYLE);
		GetWindowRect();

		// モニタのワークエリアを取得（モニタ全体を使いたければ rcMonitor を使う）
		HMONITOR hMon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hMon, &mi);
		RECT rc = mi.rcMonitor; // 全画面
		//RECT rc = mi.rcWork; // タスクバーを除いたワークエリア

		// スタイルを取り除く
		SetWindowLongPtr(hwnd_, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		SetWindowLongPtr(hwnd_, GWL_EXSTYLE, prevExStyle_ & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

		// 大きさと位置をモニタ全面に
		SetWindowPos(hwnd_, HWND_TOP,
			rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		isFullscreen_ = true;
	}
	else {
		// フルスクリーン -> ウィンドウモードに戻す
		SetWindowLongPtr(hwnd_, GWL_STYLE, prevStyle_);
		SetWindowLongPtr(hwnd_, GWL_EXSTYLE, prevExStyle_);
		SetWindowPos(hwnd_, HWND_TOP,
			wrc_.left, wrc_.top,
			wrc_.right,
			wrc_.bottom,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		isFullscreen_ = false;
	}
}

void TakeC::WinApp::ToggleFullscreen() {
	SetFullscreen(!isFullscreen_);
}