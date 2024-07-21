#include "Input.h"
#include <cassert>

void Input::Initialize() {
	HRESULT result;

	result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
		assert(SUCCEEDED(result));
}

void Input::Update() {
}
