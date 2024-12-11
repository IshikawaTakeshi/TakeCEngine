#pragma once
#include "Mesh/Mesh.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

class SkinMesh : public Mesh {
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
	void InitializeVertexResourceModel(ID3D12Device* device, ModelData modelData) override;

	/// <summary>
	/// モデルのIndexResource初期化
	/// </summary>
	/// <param name="device"></param>
	/// <param name="modelData"></param>
	void InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData) override;


	//================================= VertexBufferResource ==================================//

	/// <summary>
	/// 球体の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSphere(ID3D12Device* device) override;

	/// <summary>
	/// スプライトの頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSprite(ID3D12Device* device,Vector2 anchorPoint) override;

	/// <summary>
	/// 三角形の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceTriangle(ID3D12Device* device) override;

	//================================= IndexBufferResource ==================================//

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device) override;

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device) override;

	
public: //getter

	/// 頂点バッファビューの取得
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(int index) const { return vertexBufferViews_[index]; }

	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBufferView() const { return vertexBufferViews_; }

private:

	//頂点バッファビュー
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_{};
	
};