#pragma once
#include "../Include/ResourceDataStructure.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>

struct ModelData;
class Material;
class DirectXCommon;
class Mesh {
public:

	Mesh() = default;
	~Mesh() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void InitializeMesh(uint32_t index, DirectXCommon* dxCommon, bool enableLight, const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	//================================= VertexBufferResource ==================================//

	/// <summary>
	/// 球体の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトの頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceSprite(ID3D12Device* device);

	/// <summary>
	/// 三角形の頂点バッファリソース初期化
	/// </summary>
	void InitializeVertexResourceTriangle(ID3D12Device* device);

	/// <summary>
	/// objモデルの頂点バッファリソース初期化
	/// </summary>
	/// <param name="device"></param>
	void InitializeVertexResourceObjModel(ID3D12Device* device,ModelData modelData);

	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }


	//================================= IndexBufferResource ==================================//

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device);

	/// <summary>
	/// インデックスバッファビューの取得
	/// </summary>
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return indexBufferView_; }

	//================================= MaterialBufferResource ==================================//

	Material* GetMaterial() { return material_; }

public:

	//球体の分割数
	static inline const uint32_t kSubdivision = 16;

private:

	//マテリアル
	Material* material_;

	//頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//IndexBufferView用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

};

