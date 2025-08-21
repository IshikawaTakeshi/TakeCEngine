#pragma once
#include "application/Provider/BaseInputProvider.h"

class Player; // 前方宣言
class PlayerInputProvider : public baseInputProvider {

public:
    PlayerInputProvider(Player* player): player_(player) {}
    ~PlayerInputProvider() override = default;
    // IMoveDirectionProviderの実装
    Vector3 GetMoveDirection() const override;
	void RequestAttack() override;
	void RequestChargeAttack() override;

private:

	Player* player_; // Playerオブジェクトへのポインタ
};

