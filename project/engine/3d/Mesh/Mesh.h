#pragma once
#include "ResourceDataStructure.h"
#include "Material.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

class DirectXCommon;
//==================================================================
// サブメッシュ管理クラス
//==================================================================
class ModelMesh {
public:

	//球体の分割数
	static inline const uint32_t kSubdivision = 16;

	struct SubMesh {
		uint32_t vertexStart = 0; //頂点の開始インデックス
		uint32_t vertexCount = 0; //頂点数
		uint32_t indexStartIndex = 0; //インデックスの開始インデックス
		uint32_t indexCount = 0; //インデックス数
		std::vector<VertexData> vertices; //頂点データ
		std::vector<uint32_t> indices; //インデックスデータ
		std::unique_ptr<Material> material_;
	};
public:

	ModelMesh() = default;
	~ModelMesh();

	void InitializeMesh(DirectXCommon* dxCommon, ModelData* modelData);

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
	void InitializeInputVertexResourceModel(ID3D12Device* device, ModelData* modelData);
	void InitializeOutputVertexResourceModel(ID3D12Device* device, ModelData* modelData, ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device);

	void InitializeIndexResourceModel(ID3D12Device* device, ModelData* modelData);

	//Map
	void MapInputVertexResource(ModelData* modelData);

public: //getter
	
	/// 頂点リソースの取得
	ID3D12Resource* GetInputVertexResource() { return inputVertexResource_.Get(); }
	ID3D12Resource* GetOutputVertexResource() { return outputVertexResource_.Get(); }
	/// 頂点数リソースの取得
	ID3D12Resource* GetVertexCountResource() { return vertexCountResource_.Get(); }

	/// 頂点バッファビューの取得
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }

	/// インデックスバッファビューの取得
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return indexBufferView_; }

	std::vector<SubMesh>& GetSubMeshes() { return subMeshes_; }

protected:

	std::vector<SubMesh> subMeshes_;     //サブメッシュのコンテナ

	uint32_t vertexOffset_ = 0; //頂点オフセット
	uint32_t indexOffset_ = 0;  //インデックスオフセット

	//頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> inputVertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	//頂点数リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexCountResource_;

	//IndexBufferView用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
};