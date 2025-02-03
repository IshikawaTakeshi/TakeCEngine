#include "Command.h"
#include "Player/Player.h"

void MoveRightCommand::Execute(Player& player) {
	player.MoveRight();
}

void MoveLeftCommand::Execute(Player& player) {
	player.MoveLeft();
}
