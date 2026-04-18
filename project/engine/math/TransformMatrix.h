#pragma once
#include "Matrix4x4.h"

//============================================================================
// TransformMatrix struct
//============================================================================

// ワールド・ビュー・プロジェクション行列などをまとめた構造体
struct TransformMatrix {
	Matrix4x4 WVP;                   // ワールド・ビュー・プロジェクション行列
	Matrix4x4 World;                 // ワールド行列
	Matrix4x4 WorldInverseTranspose; // ワールド行列の逆転置行列
};