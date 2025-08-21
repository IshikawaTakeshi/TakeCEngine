#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Quaternion.h"

class baseInputProvider {
public:
		virtual ~baseInputProvider() = default;
		// 移動方向を取得する純粋仮想関数
		virtual Vector3 GetMoveDirection() const = 0;

		virtual void RequestAttack() = 0;
		virtual void RequestChargeAttack() = 0;
};