#pragma once

#include "../MyMath/VertexData.h"
#include "../Vector2.h"
#include "../Vector3.h"
#include "../Vector4.h"
#include "../MyMath/Matrix4x4.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>

class DirectXCommon;
struct ModelData {

	std::vector<VertexData> vertices;
};

class Model {
public:

	Model() = default;
	~Model() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void DrawCall(DirectXCommon* dxCommon);

	/// <summary>
	/// VertexData初期化
	/// </summary>
	void InitializeVertexData(DirectXCommon* dxCommon);




	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	
private:

	ModelData modelData_; //構築するModelData

	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	//頂点データ
	VertexData* vertexData_ = nullptr;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

};

