#pragma once
#include "application/Provider/BaseInputProvider.h"

class Player; // 前方宣言

//============================================================================
// PlayerInputProvider class
//============================================================================
class PlayerInputProvider : public baseInputProvider {
public:

	PlayerInputProvider(Player* player) : player_(player) {}
	~PlayerInputProvider() override = default;

	//移動方向の取得
	Vector3 GetMoveDirection() const override;
	//ジャンプ入力の取得
	bool IsJumpRequested() const override;
	//攻撃入力の取得
	void RequestAttack() override;
	//チャージ攻撃入力の取得
	void RequestChargeAttack() override;

private:

	Player* player_; // Playerオブジェクトへのポインタ
};