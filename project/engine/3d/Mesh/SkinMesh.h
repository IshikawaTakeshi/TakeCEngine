#pragma once
#include "Mesh/Mesh.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

class SkinMesh : public Mesh{
public:

	SkinMesh() = default;
	~SkinMesh();

	/// <summary>
	/// 頂点バッファビューの追加
	/// </summary>
	/// <param name="vbv"></param>
	void AddVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vbv) { vertexBufferViews_.push_back(vbv); }

	void SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot)override;

	/// <summary>
	/// モデルの頂点バッファリソース初期化
	/// </summary>
	/// <param name="device"></param>
	/// /// <param name="modelData"></param>
	virtual void InitializeVertexResourceModel(ID3D12Device* device, ModelData modelData);

	/// <summary>
	/// モデルのIndexResource初期化
	/// </summary>
	/// <param name="device"></param>
	/// <param name="modelData"></param>
	virtual void InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData);

	
public: //getter

	/// 頂点バッファビューの取得
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(int index) const { return vertexBufferViews_[index]; }

	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBufferView() const { return vertexBufferViews_; }

private:

	//頂点バッファビュー
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_{};
	
};