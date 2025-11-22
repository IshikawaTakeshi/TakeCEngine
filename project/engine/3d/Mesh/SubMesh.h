#pragma once
#include <cstdint>

//=============================================
// SubMesh.h
//=============================================

struct SubMesh {

	uint32_t indexCount;        // インデックス数
	uint32_t indexStart;        // インデックスバッファの開始位置
	uint32_t vertexCount;       // 頂点数
	uint32_t vertexStart;       // 頂点バッファの開始位置
	int32_t baseVertexLocation; // 頂点バッファのベース位置
};