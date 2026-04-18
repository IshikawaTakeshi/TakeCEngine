#pragma once
#include "engine/math/Vector2.h"
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"

//=================================================
// Vertexdata.h
//=================================================

//頂点データ構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};