#pragma once
#include "engine/Input/InputMapper.h"
#include "application/Provider/BaseInputProvider.h"
#include <memory>

class Player; // 前方宣言

//============================================================================
// PlayerInputProvider class
//============================================================================
class PlayerInputProvider : public baseInputProvider {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	/// <param name="player"></param>
	PlayerInputProvider(Player* player);
	~PlayerInputProvider() override = default;

	//移動方向の取得
	Vector3 GetMoveDirection() const override;
	//ジャンプ入力の取得
	bool RequestJumpInput() const override;
	//攻撃入力の取得
	bool RequestAttack(CharacterActionInput attackButton) override;
	//チャージ攻撃入力の取得
	bool RequestChargeAttack(CharacterActionInput attackButton) override;
	//ステップブースト入力の取得
	bool RequestStepBoost() const override;

private:
	//--------- variables ----------------------------------------------------

	Player* player_; // Playerオブジェクトへのポインタ
	std::unique_ptr<InputMapper<CharacterActionInput>> mapper_; // 入力マッパー
};