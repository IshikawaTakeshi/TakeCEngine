#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Quaternion.h"

//============================================================================
// baseInputProvider class
//============================================================================
class baseInputProvider {
public:
		virtual ~baseInputProvider() = default;

		// 移動方向を取得
		virtual Vector3 GetMoveDirection() const = 0;
		// カメラの向きを取得
		virtual bool IsJumpRequested() const = 0;
		// 攻撃入力を取得
		virtual void RequestAttack() = 0;
		// チャージ攻撃入力を取得
		virtual void RequestChargeAttack() = 0;
};