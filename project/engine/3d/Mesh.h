#pragma once
#include "ResourceDataStructure.h"
#include "Material.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>
#include <memory>

class DirectXCommon;
class Mesh {
public:

	enum class MeshAttribute {
		Rigid,
		Skinned
	};

	Mesh() = default;
	~Mesh();

	/// <summary>
	/// 初期化
	/// </summary>
	void InitializeMesh(DirectXCommon* dxCommon,const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	//void Update();

	//================================= VertexBufferResource ==================================//

	/// <summary>
	/// 球体の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトの頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSprite(ID3D12Device* device,Vector2 anchorPoint);

	/// <summary>
	/// 三角形の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceTriangle(ID3D12Device* device);

	/// <summary>
	/// objモデルの頂点バッファリソース初期化
	/// </summary>
	/// <param name="device"></param>
	void InitializeVertexResourceModel(ID3D12Device* device,ModelData modelData);

	void InitializeVertexResourceAABB(ID3D12Device* device);

	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }
	ID3D12Resource* GetVertexResource() { return vertexResource_.Get(); }


	//================================= IndexBufferResource ==================================//

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device);

	void InitializeIndexResourceAABB(ID3D12Device* device);

	void InitializeIndexResourceModel(ID3D12Device* device, ModelData modelData);

	/// <summary>
	/// インデックスバッファビューの取得
	/// </summary>
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return indexBufferView_; }

	//================================= MaterialBufferResource ==================================//

	Material* GetMaterial() { return material_.get(); }

public:

	//球体の分割数
	static inline const uint32_t kSubdivision = 16;

private:

	//マテリアル
	std::unique_ptr<Material> material_ = nullptr;

	//頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//IndexBufferView用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

};