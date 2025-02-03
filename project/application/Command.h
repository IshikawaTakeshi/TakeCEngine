#pragma once

class Player;
class ICommand {
public:
	virtual ~ICommand() {}
	virtual void Execute(Player& player) = 0;

};

class MoveRightCommand : public ICommand {
public:
	void Execute(Player& player) override;

};

class MoveLeftCommand : public ICommand {

public:
	void Execute(Player& player) override;
};