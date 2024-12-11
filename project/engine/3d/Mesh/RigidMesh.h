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
	void SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot) override;

	void AddVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vbv) override;

	//================================= VertexBufferResource ==================================//

	/// <summary>
	/// 球体の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSphere(ID3D12Device* device) override;

	/// <summary>
	/// スプライトの頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint) override;

	/// <summary>
	/// 三角形の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceTriangle(ID3D12Device* device) override;

	/// <summary>
	/// モデルの頂点バッファリソース初期化
	/// </summary>
	/// <param name="device"></param>
	void InitializeVertexResourceModel(ID3D12Device* device, ModelData modelData) override;

	//================================= IndexBufferResource ==================================//

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device) override;

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device) override;

	void InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData) override;

private:


	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
};