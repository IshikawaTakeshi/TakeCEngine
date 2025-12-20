#pragma once
#include "base/DirectXCommon.h"

//============================================================
// プリミティブメッシュ構造体
//============================================================

struct PrimitiveMesh {
	ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
};