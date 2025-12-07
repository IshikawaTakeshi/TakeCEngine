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

		//正規化
		if (value.Length() > 1.0f) {
			value = value.Normalize();
		}
		// スケーリング
		value.x *= mouseMoveScale_;
		value.y *= mouseMoveScale_;
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
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->PressMouse(1);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->PressMouse(0);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->PushKey(DIK_1) || Input::GetInstance()->PressMouse(3);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->PushKey(DIK_2) || Input::GetInstance()->PressMouse(4);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->PressMouse(2);
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
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->TriggerMouse(1);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->TriggerMouse(0);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->TriggerKey(DIK_1) || Input::GetInstance()->TriggerMouse(3);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->TriggerKey(DIK_2) || Input::GetInstance()->TriggerMouse(4);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->TriggerMouse(2);
	default:
		return false;
	}
}

//=============================================================================
// 単入力取得(リリース)
//=============================================================================

bool PlayerKeyInput::IsReleased(CharacterActionInput button) const {
	switch (button) {
	case CharacterActionInput::JUMP:        return Input::GetInstance()->ReleaseKey(DIK_SPACE);
	case CharacterActionInput::FLOATING:    return Input::GetInstance()->ReleaseKey(DIK_SPACE);
	case CharacterActionInput::STEPBOOST:   return Input::GetInstance()->ReleaseKey(DIK_LSHIFT);
	case CharacterActionInput::ATTACK_RA:   return Input::GetInstance()->ReleaseMouse(1);
	case CharacterActionInput::ATTACK_LA:   return Input::GetInstance()->ReleaseMouse(0);
	case CharacterActionInput::ATTACK_RB:   return Input::GetInstance()->ReleaseKey(DIK_1) || Input::GetInstance()->ReleaseMouse(3);
	case CharacterActionInput::ATTACK_LB:   return Input::GetInstance()->ReleaseKey(DIK_2) || Input::GetInstance()->ReleaseMouse(4);
	case CharacterActionInput::LOCKON:      return Input::GetInstance()->ReleaseMouse(2);
	default:
		return false;
	}
}