#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <cstdint>
#include <map>
#include <cmath>
#include <dinput.h>
#include <Xinput.h>
#include <Windows.h>
#include <wrl.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

//===================================================================================
// InputData.h
//===================================================================================

// Gamepadのボタンの種類を表す列挙型
enum class GamepadButtonType {
	A,          // Aボタン
	B,          // Bボタン
	X,          // Xボタン
	Y,          // Yボタン
	LB,         // LBボタン
	RB,         // RBボタン
	LT,         // LTボタン
	RT,         // RTボタン
	LeftStick,  // 左スティック押し込み
	RightStick, // 右スティック押し込み
	DPadUP,		// 十字キー上
	DPadDOWN,	// 十字キー下
	DPadLEFT,	// 十字キー左
	DPadRIGHT,	// 十字キー右
	Start,
};

// Gamepadのアナログ値の種類を表す列挙型
enum class GamepadValueType {
	LEFT_STICK,  // 左スティック
	RightStick, // 右スティック
};