#pragma once
#include <cstdint>

//=============================================
// SubMesh.h
//=============================================

namespace TakeC {
	/// <summary>
	/// SubMeshに必要な値をまとめて保持する構造体です。
	/// </summary>
	struct SubMesh {

		uint32_t indexCount;        // インデックス数
		uint32_t indexStart;        // インデックスバッファの開始位置
		uint32_t vertexCount;       // 頂点数
		uint32_t vertexStart;       // 頂点バッファの開始位置
		uint32_t materialIndex = 0; // 使用するマテリアルのインデックス
	};
}