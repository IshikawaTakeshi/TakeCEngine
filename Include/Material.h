#pragma once
#include "../Include/ResourceDataStructure.h"
#include "../MyMath/Transform.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cstdint>


class DirectXCommon;
class Texture;
class Material {
public:

	Material() = default;
	~Material() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void InitializeTexture(uint32_t index, DirectXCommon* dxCommon,  bool enableLight, const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void UpdateMaterial();

	/// <summary>
	/// マテリアルリソース初期化
	/// </summary>
	void InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device,bool enableLight);

	/// <summary>
	/// マテリアルリソースの取得
	/// </summary>
	ID3D12Resource* GetMaterialResource() { return materialResource_.Get(); }

private:

	//テクスチャ
	Texture* texture_;

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	MaterialData* materialData_ = nullptr;

	//uvTransform
	Transform uvTransform_;

};

