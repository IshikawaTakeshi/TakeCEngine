#pragma once

//	include
#include <typeinfo>
#include "engine/math/Vector2.h"

// enumのみに制約をかける
template<typename T>
concept InputEnum = std::is_enum_v<T>;

//============================================================================
//	IInputDevice class
//============================================================================
template<InputEnum Enum>
class IInputDevice {
public:

    IInputDevice() = default;
    virtual ~IInputDevice() = default;

    //--------- accessor -----------------------------------------------------

    // 連続入力
    virtual float GetVector(Enum axis)  const = 0;
	virtual Vector2 GetVector2(Enum axis) const = 0;

    // 単入力
    virtual bool IsPressed(Enum button) const = 0;
    virtual bool IsTriggered(Enum button) const = 0;
	virtual bool IsReleased(Enum button) const = 0;
};

