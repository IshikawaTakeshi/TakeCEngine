#pragma once
#include "base/ComPtrAliasTemplates.h"
#include <d3d12.h>

namespace TakeC {

//============================================================================
// 共通プリミティブメッシュ構造体
// 全プリミティブで使用する頂点バッファ・インデックスバッファ情報
//============================================================================
struct PrimitiveMesh {
	ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
};

} // namespace TakeC
