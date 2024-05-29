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

#include "Matrix4x4.h"
#include "Transform.h"
#include "VertexData.h"

class DirectXCommon;
class Triangle {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();


private:

	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Transform transform_{};
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	//頂点データ
	VertexData* vertexData_ = nullptr;
	//TransformationMatrix用の頂点データ
	Matrix4x4* transformationMatrixData_ = nullptr;
};

