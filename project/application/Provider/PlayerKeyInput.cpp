#include "PlayerKeyInput.h"
#include "engine/io/Input.h"

float PlayerKeyInput::GetVector(CharacterActionInput axis) const {
    axis;
    return 0.0f;
}

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

bool PlayerKeyInput::IsPressed(CharacterActionInput button) const {
    button;
    return false;
}

bool PlayerKeyInput::IsTriggered(CharacterActionInput button) const {
    button;
    return false;
}
