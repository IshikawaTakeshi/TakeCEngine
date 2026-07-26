#pragma once
#include "engine/math/Vector2.h"
#include "engine/math/Vector3.h"
#include "engine/math/Vector4.h"

//=================================================
// Vertexdata.h
//=================================================

namespace TakeC {
	//頂点データ構造体
	/// <summary>
	/// VertexDataに関するデータを保持する構造体です。
	/// </summary>
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
}