#pragma once
#include "BasePlayerState.h"
class PlayerStateAttack : public BasePlayerState {
public:
	PlayerStateAttack(Player* player) : BasePlayerState("Attack", player) {};
	//初期化
	void Initialize() override;
	//更新
	void Update() override;

};