#pragma once
#include "ResourceDataStructure.h"
#include "Material.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <vector>

struct ModelData;
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
		uint32_t indexStart = 0; //インデックスの開始インデックス
		uint32_t indexCount = 0; //インデックス数
		std::vector<VertexData> vertices; //頂点データ
		std::vector<uint32_t> indices; //インデックスデータ
		Material material_;
	};
public:

	ModelMesh() = default;
	~ModelMesh() = default;

	void InitBufferViews();

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
	void InitializeInputVertexResourceModel(ID3D12Device* device);
	void InitializeOutputVertexResourceModel(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 球体のIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSphere(ID3D12Device* device);

	/// <summary>
	/// スプライトのIndexResource初期化
	/// </summary>
	void InitializeIndexResourceSprite(ID3D12Device* device);

	void InitializeIndexResourceModel(ID3D12Device* device);

	//Map
	void MapInputVertexResource();

	std::vector<D3D12_VERTEX_BUFFER_VIEW> CreateSubMeshVBVs() const;
	std::vector<D3D12_INDEX_BUFFER_VIEW> CreateSubMeshIBVs() const;

public: //getter
	
	/// 頂点リソースの取得
	ID3D12Resource* GetInputVertexResource() const { return inputVertexResource_.Get(); }
	ID3D12Resource* GetOutputVertexResource() const { return outputVertexResource_.Get(); }

	/// 頂点バッファビューの取得
	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVBVs() const { return VBVs_; }
	/// インデックスバッファビューの取得
	const std::vector<D3D12_INDEX_BUFFER_VIEW>& GetIBVs() const { return IBVs_; }

	std::vector<SubMesh>& GetSubMeshes() { return subMeshes_; }

	std::vector<VertexData>& GetAllVertices() { return allVertices; }
	const std::vector<VertexData>& GetAllVertices() const { return allVertices; }

	std::vector<uint32_t>& GetAllIndices() { return allIndices; }

protected:

	std::vector<VertexData> allVertices; //全頂点データ
	std::vector<uint32_t> allIndices;    //全インデックスデータ
	std::vector<SubMesh> subMeshes_;     //サブメッシュのコンテナ

	//頂点バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> inputVertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexResource_;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VBVs_;

	//IndexBufferView用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	std::vector<D3D12_INDEX_BUFFER_VIEW> IBVs_;
};