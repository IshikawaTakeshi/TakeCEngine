#include "InputHandler.h"
#include "Input.h"

ICommand* InputHandler::HandleInput() {
    if (Input::GetInstance()->TriggerKey(DIK_A)) {
		return pressKeyA_;

	}
	else if (Input::GetInstance()->TriggerKey(DIK_D)) {
		return pressKeyD_;
	}
    return nullptr;
}

void InputHandler::AssignMoveLeftCommand2PressKeyA() {
	ICommand* command = new MoveLeftCommand();
	this->pressKeyA_ = command;
}

void InputHandler::AssignMoveRightCommand2PressKeyD() {

	ICommand* command = new MoveRightCommand();
	this->pressKeyD_ = command;
}
