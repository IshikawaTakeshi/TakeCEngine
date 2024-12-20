#pragma once
#include "ResourceDataStructure.h"
#include "Material.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

class DirectXCommon;
class Mesh {
public:

	Mesh() = default;
	~Mesh();

	void InitializeMesh(DirectXCommon* dxCommon, const std::string& filePath);

	/// <summary>
		/// 描画処理時に使用する頂点バッファを設定
		/// </summary>
		/// <param name="commandList"></param>
		/// <param name="startSlot"></param>
	void SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot);

	void AddVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vbv);


	//================================= VertexBufferResource ==================================//

	/// <summary>
	/// 球体の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトの頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint);

	/// <summary>
	/// 三角形の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceTriangle(ID3D12Device* device);

	/// <summary>
	/// モデルの頂点バッファリソース初期化
	/// </summary>
	/// <param name="device"></param>
	void InitializeVertexResourceModel(ID3D12Device* device, ModelData modelData);

	//================================= IndexBufferResource ==================================//

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device);

	void InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData);


public: //getter
	
	/// 頂点リソースの取得
	ID3D12Resource* GetVertexResource() { return vertexResource_.Get(); }

	/// 頂点バッファビューの取得
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(int num) const { return vertexBufferViews_[num]; }

	/// インデックスバッファビューの取得
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return indexBufferView_; }

	/// マテリアルの取得
	Material* GetMaterial() { return material_.get(); }

public:

	//球体の分割数
	static inline const uint32_t kSubdivision = 16;

protected:

	//マテリアル
	std::unique_ptr<Material> material_ = nullptr;

	//頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_;

	//IndexBufferView用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
};