#pragma once
#include "application/Provider/IMoveDirectionProvider.h"
#include "application/Entity/Player/Player.h"

class PlayerMoveDirectionProvider :
    public IMoveDirectionProvider {

public:
    PlayerMoveDirectionProvider(Player* player): player_(player) {}
    ~PlayerMoveDirectionProvider() override = default;
    // IMoveDirectionProviderの実装
    Vector3 GetMoveDirection() const override;

private:

	Player* player_; // Playerオブジェクトへのポインタ
};

