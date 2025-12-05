#include "PlayerKeyInput.h"
#include "engine/Input/Input.h"

//=============================================================================
// 連続入力取得
//=============================================================================
float PlayerKeyInput::GetVector(CharacterActionInput axis) const {
    axis;
    return 0.0f;
}

//=============================================================================
// 連続入力取得(Vector2)
//=============================================================================
Vector2 PlayerKeyInput::GetVector2(CharacterActionInput axis) const {
    
    Vector2 value{};

    switch (axis) {
    case CharacterActionInput::MOVE:

        if(Input::GetInstance()->PushKey(DIK_W)) {
            value.y += 1.0f;
        } else if (Input::GetInstance()->PushKey(DIK_S)) {
            value.y -= 1.0f;
		}
        if(Input::GetInstance()->PushKey(DIK_A)) {
            value.x -= 1.0f;
        } else if (Input::GetInstance()->PushKey(DIK_D)) {
            value.x += 1.0f;
        }

        break;
    case CharacterActionInput::MOVE_CAMERA:

        Input::GetInstance()->GetMouseMove();

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
    button;
    return false;
}

//=============================================================================
// 単入力取得(トリガー)
//=============================================================================
bool PlayerKeyInput::IsTriggered(CharacterActionInput button) const {
    button;
    return false;
}
