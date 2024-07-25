#include "../Include/Input.h"
#include "../Include/WinApp.h"
#include <cassert>
#include <memory.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

Input::~Input() {}

Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

void Input::Initialize(WinApp* winApp) {
	HRESULT result;

	winApp_ = winApp;

	//DirectInputの初期化
	result = DirectInput8Create(winApp->GetHInstance(),
		DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの初期化
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	//排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update() {

	HRESULT result;

	//前回のキー入力を保存
	memcpy(preKey, key, sizeof(key));
	//キーボード情報の取得開始
	result = keyboard_->Acquire();
	
	//全てのキーの入力情報を取得
	result = keyboard_->GetDeviceState(sizeof(key), key);
	
}

bool Input::PushKey(BYTE keyNumber) {
	// 指定キーが押されているか
	return key[keyNumber] != 0;
}

bool Input::TriggerKey(BYTE keyNumber) {
	// 指定キーが押された瞬間か
	return key[keyNumber] && !preKey[keyNumber];
}
