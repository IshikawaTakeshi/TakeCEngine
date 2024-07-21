#pragma once
#define DIRECTINPUT_VERSION 0x0800 //DirectInputのバージョン指定
#include <dinput.h>
#include <memory>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class Input {
public:
	void Initialize();

	void Update();

private:

	std::unique_ptr<IDirectInput8> directInput = nullptr;
	std::unique_ptr<IDirectInputDevice8> keyboard = nullptr;

};

