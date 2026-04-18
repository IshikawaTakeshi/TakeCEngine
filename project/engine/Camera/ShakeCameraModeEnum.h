#pragma once
#include <cstdint>

//============================================================================
//	Camera Shake Mode Enum
//============================================================================

enum class ShakeCameraMode : uint8_t {
	NONE, 	  //シェイクなし
	HORIZONTAL, //水平シェイク
	VERTICAL,   //垂直シェイク
	Z_SHAKE,     //前後シェイク
	BOTH,        //水平＋垂直シェイク
	FULL_SHAKE,   //全方向シェイク
};