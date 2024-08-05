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
#include "../MyMath/TransformMatrix.h"
#include "../Include/ResourceDataStructure.h"

class DirectXCommon;
class Texture;
class Mesh;
class SpriteCommon;
class Sprite {
public:

	Sprite() = default;
	~Sprite();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon, const std::string& textureFilePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// directionalLightData初期化
	/// </summary>
	void InitializeDirectionalLightData(DirectXCommon* dxCommon);

	/// <summary>
	/// 描画処理
	/// </summary>
	void DrawCall(DirectXCommon* dxCommon);

	/// <summary>
	/// Transformの取得
	/// </summary>
	Transform GetTransform() { return transform_; }


private:

	//SpriteCommon
	SpriteCommon* spriteCommon_ = nullptr;

	//メッシュ
	Mesh* mesh_ = nullptr;

	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//sprite用のTransformationMatrix用の頂点データ
	TransformMatrix* wvpData_ = nullptr;

	Transform transform_{};
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;
	
	//平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLightData* directionalLightData_ = nullptr;
};

