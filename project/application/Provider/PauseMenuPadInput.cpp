#include "PauseMenuPadInput.h"

using namespace TakeC;

float PauseMenuPadInput::GetVector(PauseMenuInput axis) const {
	switch (axis) {
	case PauseMenuInput::Up:
		return TakeC::Input::GetInstance().GetLeftStickState(0).y;
	default:
		return 0.0f;
	}
}

Vector2 PauseMenuPadInput::GetVector2(PauseMenuInput) const {
	return Vector2();
}

bool PauseMenuPadInput::IsPressed(PauseMenuInput button) const {
	switch (button) {
	case PauseMenuInput::Up: return Input::GetInstance().PushButton(0, GamepadButtonType::DPadUP);
	case PauseMenuInput::Down: return Input::GetInstance().PushButton(0, GamepadButtonType::DPadDOWN);
	case PauseMenuInput::Left: return Input::GetInstance().PushButton(0, GamepadButtonType::DPadLEFT);
	case PauseMenuInput::Right: return Input::GetInstance().PushButton(0, GamepadButtonType::DPadRIGHT);
	case PauseMenuInput::Select: return Input::GetInstance().PushButton(0, GamepadButtonType::A);
	case PauseMenuInput::Back: return Input::GetInstance().PushButton(0, GamepadButtonType::B);
	default: 
		return false;
	}
}

bool PauseMenuPadInput::IsTriggered(PauseMenuInput button) const {
	switch (button) {
	case PauseMenuInput::Up: return Input::GetInstance().TriggerButton(0, GamepadButtonType::DPadUP);
	case PauseMenuInput::Down: return Input::GetInstance().TriggerButton(0, GamepadButtonType::DPadDOWN);
	case PauseMenuInput::Left: return Input::GetInstance().TriggerButton(0, GamepadButtonType::DPadLEFT);
	case PauseMenuInput::Right: return Input::GetInstance().TriggerButton(0, GamepadButtonType::DPadRIGHT);
	case PauseMenuInput::Select: return Input::GetInstance().TriggerButton(0, GamepadButtonType::A);
	case PauseMenuInput::Back: return Input::GetInstance().TriggerButton(0, GamepadButtonType::B);
	default:
		return false;
	}
}

bool PauseMenuPadInput::IsReleased(PauseMenuInput button) const {
	switch (button) {
	case PauseMenuInput::Up: return Input::GetInstance().ReleaseButton(0, GamepadButtonType::DPadUP);
	case PauseMenuInput::Down: return Input::GetInstance().ReleaseButton(0, GamepadButtonType::DPadDOWN);
	case PauseMenuInput::Left: return Input::GetInstance().ReleaseButton(0, GamepadButtonType::DPadLEFT);
	case PauseMenuInput::Right: return Input::GetInstance().ReleaseButton(0, GamepadButtonType::DPadRIGHT);
	case PauseMenuInput::Select: return Input::GetInstance().ReleaseButton(0, GamepadButtonType::A);
	case PauseMenuInput::Back: return Input::GetInstance().ReleaseButton(0, GamepadButtonType::B);
	default:
		return false;
	}
}
