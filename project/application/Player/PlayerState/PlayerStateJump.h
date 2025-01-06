#pragma once
#include "BasePlayerState.h"

class PlayerStateJump : public BasePlayerState {
public:

	PlayerStateJump(Player* player) : BasePlayerState("Jump",player) {};
	//初期化
	void Initialize() override;
	//更新
	void Update() override;

};