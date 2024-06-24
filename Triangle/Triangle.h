#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxcapi.h>

#include <wrl.h>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "../MyMath/Matrix4x4.h"
#include "../MyMath/Transform.h"
#include "../Include/ResourceDataStructure.h"

class DirectXCommon;
class Texture;
class Triangle {
public:

	Triangle() = default;
	~Triangle();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,Matrix4x4 cameraView);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(
#ifdef _DEBUG
		int id
#endif // DEBUG	
	);

	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() {
		return vertexBufferView_;
	}

	/// <summary>
	/// MaterialData初期化
	/// </summary>
	void InitializeMaterialData(DirectXCommon* dxCommon);

	/// <summary>
	/// MaterialData初期化
	/// </summary>
	void DrawCall(DirectXCommon* dxCommon, Texture* texture);



	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexResource() {
		return vertexResource_;
	}

	/// <summary>
	/// 頂点リソースの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> GetWvpResource() {
		return wvpResource_;
	}

	void SetVertexData(VertexData* vertexData) { vertexData_ = vertexData; }

private:

	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//CPU用のTransform
	Transform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	//頂点データ
	VertexData* vertexData_ = nullptr;
	//TransformationMatrix用の頂点データ
	Matrix4x4* transformMatrixData_ = nullptr;

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Vector4* materialData_;

	//Texture
	Texture* Texture_ = nullptr;
};

