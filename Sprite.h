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
class Sprite {
public:

	Sprite() = default;
	~Sprite() = default;

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

	/// <summary>
	/// Resource生成関数
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(
		Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()
	{ return vertexBufferViewSprite_; }

	/// <summary>
	/// sprite用のTransformationMatrix用の頂点リソースの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource()
	{ return transformationMatrixResourceSprite_; }

private:
	//Sprite用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite_;
	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite_;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};

	Transform transformSprite_{};
	Matrix4x4 worldMatrixSprite_;
	Matrix4x4 viewMatrixSprite_;
	Matrix4x4 projectionMatrixSprite_;
	Matrix4x4 worldViewProjectionMatrixSprite_;

	//sprite用頂点データ
	VertexData* vertexDataSprite_ = nullptr;
	//sprite用のTransformationMatrix用の頂点データ
	Matrix4x4* transformationMatrixDataSprite_ = nullptr;
};

