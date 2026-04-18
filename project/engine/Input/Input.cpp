#include "Input.h"
#include "WinApp.h"
#include <cassert>
#include <cmath>
#include <memory.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

using namespace TakeC;

Input& Input::GetInstance() {
	static Input instance;
	return instance;
}

//=========================================================
// 初期化
//=========================================================

void Input::Initialize(TakeC::WinApp* winApp) {
	HRESULT result;

	winApp_ = winApp;

	//DirectInputの初期化
	result = DirectInput8Create(winApp->GetHInstance(),
		DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの初期化
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboardDevice_, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboardDevice_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	//排他制御レベルのセット
	result = keyboardDevice_->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	//マウス初期化
	result = directInput_->CreateDevice(GUID_SysMouse, &mouseDevice_, NULL);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));

	//gamepadの初期化
	gamePad_ = std::make_unique<GamePad>();
	gamePad_->Initialize();
	
}

//=========================================================
// 更新処理
//=========================================================

void Input::Update() {

	HRESULT result;

	//前回のキー入力を保存
	memcpy(preKey, key, sizeof(key));
	//キーボード情報の取得開始
	result = keyboardDevice_->Acquire();
	//全てのキーの入力情報を取得
	result = keyboardDevice_->GetDeviceState(sizeof(key), key);

	//マウス情報の取得
	memcpy(&mousePre_, &mouse_, sizeof(mouse_));
	result = mouseDevice_->Acquire();
	result = mouseDevice_->GetDeviceState(sizeof(DIMOUSESTATE2), &mouse_);
	//ポーリング
	result = mouseDevice_->Poll();

	//gamepadの更新
	gamePad_->Update();
}

//=========================================================
// 終了処理
//=========================================================

void Input::Finalize() {
	if (keyboardDevice_) {
		keyboardDevice_->Unacquire();
		keyboardDevice_.Reset();
	}
	if (directInput_) {
		directInput_.Reset();
	}
}

//=========================================================
// キー入力(押下)の取得
//=========================================================

bool Input::PushKey(BYTE keyNumber) {
	// 指定キーが押されているか
	return key[keyNumber] != 0;
}

//=========================================================
// キー入力(トリガー)の取得
//=========================================================

bool Input::TriggerKey(BYTE keyNumber) {
	// 指定キーが押された瞬間か
	return key[keyNumber] && !preKey[keyNumber];
}

//=========================================================
// キー入力(リリース)の取得
//=========================================================
bool Input::ReleaseKey(BYTE keyNumber) {
	return !key[keyNumber] && preKey[keyNumber];
}

//=========================================================
// ゲームパッドの入力取得
//=========================================================
bool Input::PushButton(int stickNo, GamepadButtonType button) const {
	return gamePad_->PushButton(stickNo, button);
}

bool Input::TriggerButton(int stickNo, GamepadButtonType button) const {
	return gamePad_->TriggerButton(stickNo, button);
}

bool Input::ReleaseButton(int stickNo, GamepadButtonType button) const {
	return gamePad_->ReleaseButton(stickNo, button);
}

float Input::GetTriggerValue(int stickNo, GamepadButtonType button) const {
	return gamePad_->GetTriggerValue(stickNo, button);
}

Vector2 Input::GetLeftStickState(int stickNo) const {
	return gamePad_->GetLeftStickState(stickNo);
}

Vector2 Input::GetRightStickState(int stickNo) const {
	// スティックの状況を取得
	return gamePad_->GetRightStickState(stickNo);
}

Vector2 Input::GetStickValue(int stickNo, GamepadValueType valueType) const {
	// スティックの値を取得
	return gamePad_->GetStickValue(stickNo, valueType);
}


//=========================================================
// 全マウス情報取得
//=========================================================


const DIMOUSESTATE2& Input::GetAllMouse() const {
	return mouse_;
}

//=========================================================
// マウスの押下をチェック
//=========================================================

bool Input::PressMouse(int32_t buttonNumber) const {
	return mouse_.rgbButtons[buttonNumber] & 0x80;
}

//=========================================================
// マウスのトリガーをチェック
//=========================================================

bool Input::TriggerMouse(int32_t buttonNumber) const {
	return (mouse_.rgbButtons[buttonNumber] & 0x80) && !(mousePre_.rgbButtons[buttonNumber] & 0x80);
}

//=========================================================
// マウスのリリースをチェック
//=========================================================
bool Input::ReleaseMouse(int32_t buttonNumber) const {
	return !(mouse_.rgbButtons[buttonNumber] & 0x80) && (mousePre_.rgbButtons[buttonNumber] & 0x80);
}
//=========================================================
// マウス移動量を取得
//=========================================================



Input::MouseMove Input::GetMouseMove() {
	MouseMove move;
	move.lX = mouse_.lX;
	move.lY = mouse_.lY;
	move.lZ = mouse_.lZ;
	return move;
}
//=========================================================
// ホイールスクロール量を取得する
//=========================================================

int32_t Input::GetWheel() const {
	return mouse_.lZ;
}

//=========================================================
// マウスの位置を取得する（ウィンドウ座標系）
//=========================================================

const Vector2& Input::GetCursorPosition() const {
    static POINT point;
    point.x = static_cast<LONG>(mousePosition_.x);
    point.y = static_cast<LONG>(mousePosition_.y);
    GetCursorPos(&point);
    ScreenToClient(winApp_->GetHwnd(), &point);
    static Vector2 updatedMousePosition;
    updatedMousePosition.x = static_cast<float>(point.x);
    updatedMousePosition.y = static_cast<float>(point.y);
    return updatedMousePosition;
}
