#pragma once
#include "BasePlayerState.h"
class PlayerStateIdle : public BasePlayerState {
public:
	PlayerStateIdle(Player* player) : BasePlayerState("Idle",player) {};
	//初期化
	void Initialize() override;
	//更新
	void Update() override;

};

