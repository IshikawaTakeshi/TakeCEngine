#include "../Include/Input.h"
#include "../Include/WinApp.h"
#include <cassert>

Input::~Input() {}

void Input::Initialize(WinApp* winApp) {
	HRESULT result;

	//DirectInputの初期化
	result = DirectInput8Create(winApp->GetWndClass().hInstance,
		DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの初期化
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	//排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(winApp->GetHwnd(),DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	assert(SUCCEEDED(result));
}

void Input::Update() {

	HRESULT result;

	//前回のキー入力を保存
	memcpy(preKey, key, sizeof(key));

	//キーボード情報の取得開始
	result = keyboard_->Acquire();
	assert(SUCCEEDED(result));

	//全てのキーの入力情報を取得
	result = keyboard_->GetDeviceState(sizeof(key), key);
	assert(SUCCEEDED(result));

}

bool Input::PushKey(BYTE keyNumber) {
	
	//指定キーが押されているか
	if (key[keyNumber]) {
		return true;
	}

	//そうでなければfalseを返す
	return false;
}

bool Input::TriggerKey(BYTE keyNumber) {

	if (key[keyNumber] && !preKey[keyNumber]) {
		return true;
	}

	return false;
}
