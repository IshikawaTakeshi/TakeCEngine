#include "Input.h"
#include "WinApp.h"
#include <cassert>
#include <memory.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")


Input* Input::instance_ = nullptr;

Input* Input::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new Input();
	}
	return instance_;
}

void Input::Initialize(WinApp* winApp) {
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

	//ジョイスティックの初期化
	devJoysticks_.resize(4);
	for (size_t i = 0; i < devJoysticks_.size(); ++i) {
		devJoysticks_[i].type_ = PadType::XInput; // 初期化
	}
}

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

	// ジョイスティック情報の更新
	for (size_t i = 0; i < devJoysticks_.size(); ++i) {
		if (devJoysticks_[i].type_ == PadType::XInput) {
			XINPUT_STATE state = {};
			if (XInputGetState(static_cast<DWORD>(i), &state) == ERROR_SUCCESS) {
				devJoysticks_[i].state_.xInput_ = state;
			} else {
				memset(&devJoysticks_[i].state_.xInput_, 0, sizeof(XINPUT_STATE));
			}
		}
	}
}

void Input::Finalize() {
	if (keyboardDevice_) {
		keyboardDevice_->Unacquire();
		keyboardDevice_.Reset();
	}
	if (directInput_) {
		directInput_.Reset();
	}
	delete instance_;
	instance_ = nullptr;
}

bool Input::PushKey(BYTE keyNumber) {
	// 指定キーが押されているか
	return key[keyNumber] != 0;
}

bool Input::TriggerKey(BYTE keyNumber) {
	// 指定キーが押された瞬間か
	return key[keyNumber] && !preKey[keyNumber];
}

bool Input::GetJoystickState(int32_t stickNo, DIJOYSTATE2& out) const {

	if (stickNo < 0 || stickNo >= static_cast<int32_t>(devJoysticks_.size())) return false;

	auto& joystick = devJoysticks_[stickNo];
	if (!joystick.device_) return false;

	out = joystick.state_.directInput_;
	return true;
}

bool Input::GetJoystickStatePrevious(int32_t stickNo, DIJOYSTATE2& out) const {
	if (stickNo < 0 || stickNo >= devJoysticks_.size()) return false;
	if (devJoysticks_[stickNo].type_ != PadType::DirectInput) return false;
	out = devJoysticks_[stickNo].statePre_.directInput_;
	return true;
}

bool Input::GetJoystickState(int32_t stickNo, XINPUT_STATE& out) const {
	if (stickNo < 0 || stickNo >= devJoysticks_.size()) return false;
	if (devJoysticks_[stickNo].type_ != PadType::XInput) return false;
	out = devJoysticks_[stickNo].state_.xInput_;
	return true;
}

bool Input::GetJoystickStatePrevious(int32_t stickNo, XINPUT_STATE& out) const {
	if (stickNo < 0 || stickNo >= devJoysticks_.size()) return false;
	if (devJoysticks_[stickNo].type_ != PadType::XInput) return false;
	out = devJoysticks_[stickNo].statePre_.xInput_;
	return true;
}

void Input::SetJoystickDeadZone(int32_t stickNo, int32_t deadZoneL, int32_t deadZoneR) {
	if (stickNo < 0 || stickNo >= devJoysticks_.size()) return;
	devJoysticks_[stickNo].deadZoneL_ = deadZoneL;
	devJoysticks_[stickNo].deadZoneR_ = deadZoneR;
}

size_t Input::GetNumberOfJoysticks() {
	return devJoysticks_.size();
}

const DIMOUSESTATE2& Input::GetAllMouse() const {
	return mouse_;
}

bool Input::IsPressMouse(int32_t buttonNumber) const {
	return mouse_.rgbButtons[buttonNumber] & 0x80;
}

bool Input::IsTriggerMouse(int32_t buttonNumber) const {
	return (mouse_.rgbButtons[buttonNumber] & 0x80) && !(mousePre_.rgbButtons[buttonNumber] & 0x80);
}

Input::MouseMove Input::GetMouseMove() {
	MouseMove move;
	move.lX = mouse_.lX;
	move.lY = mouse_.lY;
	move.lZ = mouse_.lZ;
	return move;
}

int32_t Input::GetWheel() const {
	return mouse_.lZ;
}

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
