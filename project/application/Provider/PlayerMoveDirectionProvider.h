#pragma once
#include "application/Provider/IMoveDirectionProvider.h"

class Player; // 前方宣言
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

