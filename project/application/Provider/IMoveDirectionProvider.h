#pragma once
#include "engine/math/Vector3.h"
#include "engine/math/Quaternion.h"

class IMoveDirectionProvider {
public:
		virtual ~IMoveDirectionProvider() = default;
		// 移動方向を取得する純粋仮想関数
		virtual Vector3 GetMoveDirection() const = 0;

};

