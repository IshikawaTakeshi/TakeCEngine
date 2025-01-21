#pragma once
#include "BasePlayerState.h"
class PlayerStateRun : public BasePlayerState {
public:
	PlayerStateRun(Player* player) : BasePlayerState("Run",player) {};
	//初期化
	void Initialize() override;
	//更新
	void Update() override;

};

