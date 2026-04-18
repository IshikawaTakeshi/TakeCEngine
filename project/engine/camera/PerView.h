#pragma once
#include "engine/math/Matrix4x4.h"

//=============================================================================
// PerView struct
//=============================================================================

//View情報
struct PerView {
	Matrix4x4 viewProjection;  //ビュー射影行列
	Matrix4x4 billboardMatrix; //ビルボード行列
	bool isBillboard;          //ビルボードかどうか
};