#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <dinput.h>
#include <wrl.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//エイリアステンプレート
template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

class WinApp;
class Input {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="winApp"></param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="winApp"></param>
	void Update();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

private:

	ComPtr<IDirectInput8> directInput_ = nullptr;
	ComPtr<IDirectInputDevice8> keyboard_ = nullptr;

	//WindowsAPI
	WinApp* winApp_ = nullptr;
	//キー情報
	BYTE key[256] = {};
	//前回フレームのキーデータ
	BYTE preKey[256] = {};

};

