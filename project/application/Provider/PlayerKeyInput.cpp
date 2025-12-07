#include "PlayerKeyInput.h"
#include "engine/Input/Input.h"

//=============================================================================
// 連続入力取得(float)
//=============================================================================
float PlayerKeyInput::GetVector(CharacterActionInput axis) const {
	switch (axis) {
	case CharacterActionInput::MOVE:
		// 縦軸のみ必要なときに使う例
		if (Input::GetInstance()->PushKey(DIK_W)) return 1.0f;
		if (Input::GetInstance()->PushKey(DIK_S)) return -1.0f;
		return 0.0f;
	default:
		return 0.0f;
	}
}

//=============================================================================
// 連続入力取得(Vector2)
//=============================================================================
Vector2 PlayerKeyInput::GetVector2(CharacterActionInput axis) const {

	Vector2 value{};
	switch (axis) {
	case CharacterActionInput::MOVE:
		if (Input::GetInstance()->PushKey(DIK_W)) value.y += 1.0f;
		if (Input::GetInstance()->PushKey(DIK_S)) value.y -= 1.0f;
		if (Input::GetInstance()->PushKey(DIK_A)) value.x -= 1.0f;
		if (Input::GetInstance()->PushKey(DIK_D)) value.x += 1.0f;
		break;
	case CharacterActionInput::MOVE_CAMERA:
		// ここではマウス移動量を直接返す例
	{
		auto move = Input::GetInstance()->GetMouseMove();
		value.x = static_cast<float>(move.lX);
		value.y = static_cast<float>(move.lY);
	}
	break;
	default:
		break;
	}
	return value;
}

//=============================================================================
// 単入力取得(押下)
//=============================================================================
bool PlayerKeyInput::IsPressed(CharacterActionInput button) const {
	switch (button) {
	case CharacterActionInput::JUMP:        return Input::GetInstance()->PushKey(DIK_SPACE);
	case CharacterActionInput::FLOATING:    return Input::GetInstance()->PushKey(DIK_SPACE);
	case CharacterActionInput::STEPBOOST:   return Input::GetInstance()->PushKey(DIK_LSHIFT);
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->IsPressMouse(0);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->IsPressMouse(1);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->PushKey(DIK_1) || Input::GetInstance()->IsPressMouse(3);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->PushKey(DIK_2) || Input::GetInstance()->IsPressMouse(4);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->IsPressMouse(2);
	default:
		return false;
	}
}

//=============================================================================
// 単入力取得(トリガー)
//=============================================================================
bool PlayerKeyInput::IsTriggered(CharacterActionInput button) const {
	switch (button) {
	case CharacterActionInput::JUMP:        return Input::GetInstance()->TriggerKey(DIK_SPACE);
	case CharacterActionInput::FLOATING:    return Input::GetInstance()->TriggerKey(DIK_SPACE);
	case CharacterActionInput::STEPBOOST:   return Input::GetInstance()->TriggerKey(DIK_LSHIFT);
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->TriggerKey(DIK_J);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->TriggerKey(DIK_K);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->TriggerKey(DIK_U);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->TriggerKey(DIK_I);
	default:
		return false;
	}
}
