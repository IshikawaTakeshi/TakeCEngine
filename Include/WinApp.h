#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class WinApp {
public:

	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;


	WinApp();
	~WinApp();

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>シングルトンインスタンス</returns>
	static WinApp* GetInstance();

	/// <summary>
	/// ウィンドウプロシージャ
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="msg">メッセージ番号</param>
	/// <param name="wparam">メッセージ情報1</param>
	/// <param name="lparam">メッセージ情報2</param>
	/// <returns>成否</returns>
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// ゲームウィンドウの作成
	/// </summary>
	void CreateGameWindow(const wchar_t title[]);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const wchar_t title[]);

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// メッセージの処理
	/// </summary>
	/// <returns>終了かどうか</returns>
	bool ProcessMessage();

public:

	/// <summary>
	/// ウィンドウクラスのhInstance取得
	/// </summary>
	/// <returns></returns>
	const HINSTANCE& GetHInstance() const { return wc_.hInstance; }

	/// <summary>
	/// ウィンドウハンドルの取得
	/// </summary>
	/// <returns></returns>
	const HWND& GetHwnd() const { return hwnd_; }

	const RECT& GetWindowRect() const { return wrc_; }

private:

	
	//ウィンドウクラス
	WNDCLASS wc_;

	RECT wrc_;
	//ウィンドウハンドル
	HWND hwnd_;
	//debugController
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_;
	
	
};

