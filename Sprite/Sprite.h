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
#include "../MyMath/VertexData.h"
#include "../MyMath/Material.h"
#include "../MyMath/TransformMatrix.h"
#include "../DirectionalLight.h"

class DirectXCommon;
class Texture;
class Sprite {
public:

	Sprite() = default;
	~Sprite();

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
	/// MaterialData初期化
	/// </summary>
	void InitializeMaterialData(DirectXCommon* dxCommon);

	/// <summary>
	/// directionalLightData初期化
	/// </summary>
	void InitializeDirectionalLightData(DirectXCommon* dxCommon);

	/// <summary>
	/// IndexBufferView初期化
	/// </summary>
	void InitializeIndexBufferView(DirectXCommon* dxCommon);


	/// <summary>
	/// MaterialData初期化
	/// </summary>
	void InitializeCommandList(DirectXCommon* dxCommon, Texture* texture);


	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()
	{ return vertexBufferView_; }

	/// <summary>
	/// sprite用のTransformationMatrix用の頂点リソースの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTransformationMatrixResource()
	{ return transformationMatrixResource_; }

	/// <summary>
	/// Transformの取得
	/// </summary>
	Transform GetTransform() { return transform_; }


private:
	//Sprite用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Transform transform_{};
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	//sprite用頂点データ
	VertexData* vertexData_ = nullptr;
	//sprite用のTransformationMatrix用の頂点データ
	TransformMatrix* transformMatrixData_ = nullptr;

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite_;
	Material* materialDataSprite_ = nullptr;
	//平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLight* directionalLightData_ = nullptr;
	//IndexBufferView用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

};

