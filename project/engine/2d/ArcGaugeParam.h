#pragma once
#include "engine/Math/Vector2.h"
#include <json.hpp>

struct ArcGaugeParam {
    Vector2 center = { 0.0f,0.0f }; // 中心座標
    float fillAmount  = 0.0f; // 塗りつぶし量
    float innerRadius = 0.0f; // 内半径
    float outerRadius = 1.0f; // 外半径
    float startAngle  = 0.0f; // 開始角度
    float arcRange    = 0.0f; // 弧の最大角度
    int clockwise     = 0;    // 回転方向
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    ArcGaugeParam, 
    center,
    fillAmount,
    innerRadius,
    outerRadius,
    startAngle, 
    arcRange,
    clockwise)