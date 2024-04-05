#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp {
public:

	WinApp();
	~WinApp();
	void Init();

	bool ProcessMessage();

	static WinApp* GetInstance();

	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	//ウィンドウクラス
	WNDCLASS wc;

	RECT wrc;

	//ウィンドウハンドル
	HWND hwnd;

	//ウィンドウプロシージャ
	static LRESULT WindowProc(HWND hwmd, UINT msg,
		WPARAM wparam, LPARAM lparam);

	//ウィンドウの生成
	void CreateGameWindow();
};

