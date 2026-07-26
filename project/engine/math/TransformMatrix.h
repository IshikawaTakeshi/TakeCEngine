#pragma once
#include "Matrix4x4.h"

//============================================================================
// TransformMatrix struct
//============================================================================

// ワールド・ビュー・プロジェクション行列などをまとめた構造体
/// <summary>
/// TransformMatrixの数学的な値と変換情報を表す構造体です。
/// </summary>
struct TransformMatrix {
	Matrix4x4 WVP;                   // ワールド・ビュー・プロジェクション行列
	Matrix4x4 World;                 // ワールド行列
	Matrix4x4 WorldInverseTranspose; // ワールド行列の逆転置行列
};