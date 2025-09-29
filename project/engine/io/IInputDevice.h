#pragma once

//	include
#include <typeinfo>

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

    // 単入力
    virtual bool IsPressed(Enum button) const = 0;
    virtual bool IsTriggered(Enum button) const = 0;
};

