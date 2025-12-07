#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Quaternion.h"
#include "application/Provider/Enum/CharacterActionInputEnum.h"

//============================================================================
// baseInputProvider class
//============================================================================
class baseInputProvider {
public:
		virtual ~baseInputProvider() = default;

		// 移動方向を取得
		virtual Vector3 GetMoveDirection() const = 0;
		// カメラの向きを取得
		virtual bool RequestJumpInput() const = 0;
		// 攻撃入力を取得
		virtual bool RequestAttack(CharacterActionInput attackButton) = 0;
		// チャージ攻撃入力を取得
		virtual bool RequestChargeAttack(CharacterActionInput attackButton) = 0;
		//ステップブースト入力を取得
		virtual bool RequestStepBoost() const = 0;
};