#pragma once
#include "Mesh/Mesh.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

class RigidMesh : public Mesh {
public:

	RigidMesh() = default;
	~RigidMesh();

	/// <summary>
	/// 描画処理時に使用する頂点バッファを設定
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="startSlot"></param>
	virtual void SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot) override;

	virtual void AddVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vbv) override;

	//================================= VertexBufferResource ==================================//

	/// <summary>
	/// 球体の頂点バッファリソース初期化
	/// </summary>
	virtual void InitializeVertexResourceSphere(ID3D12Device* device) override;

	/// <summary>
	/// スプライトの頂点バッファリソース初期化
	/// </summary>
	virtual void InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint) override;

	/// <summary>
	/// 三角形の頂点バッファリソース初期化
	/// </summary>
	virtual void InitializeVertexResourceTriangle(ID3D12Device* device) override;

	/// <summary>
	/// モデルの頂点バッファリソース初期化
	/// </summary>
	/// <param name="device"></param>
	virtual void InitializeVertexResourceModel(ID3D12Device* device, ModelData modelData) override;

	//================================= IndexBufferResource ==================================//

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	virtual void InitializeIndexResourceSphere(ID3D12Device* device) override;

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	virtual void InitializeIndexResourceSprite(ID3D12Device* device) override;

	virtual void InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData) override;

private:


	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};