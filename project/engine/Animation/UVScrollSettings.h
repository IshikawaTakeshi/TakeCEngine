#pragma once
#include "engine/math/Vector2.h"
#include <json.hpp>

//============================================================================
// UVスクロール設定
//============================================================================
struct UVScrollSettings {
	Vector2 scrollSpeed = { 0.0f, 0.0f }; // スクロール速度（UV単位/秒）
	bool wrapU = true;  // U座標をラップするか
	bool wrapV = true;  // V座標をラップするか
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UVScrollSettings, scrollSpeed, wrapU, wrapV)