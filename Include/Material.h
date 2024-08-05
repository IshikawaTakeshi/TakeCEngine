#pragma once
#include "../Include/ResourceDataStructure.h"
#include "../MyMath/Transform.h"
#include "../Texture/Texture.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cstdint>


class DirectXCommon;
class Material {
public:

	Material() = default;
	~Material();

	/// <summary>
	/// 初期化
	/// </summary>
	void InitializeTexture(uint32_t index, DirectXCommon* dxCommon, const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void UpdateMaterialImGui();

	/// <summary>
	/// マテリアルリソース初期化
	/// </summary>
	void InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device);

public: //ゲッター

	/// <summary>
	/// マテリアルリソースの取得
	/// </summary>
	ID3D12Resource* GetMaterialResource() { return materialResource_.Get(); }

	Texture* GetTexture() { return texture_; }

public: //セッター

	/// <summary>
	///uvTransformのセッター
	/// </summary>
	void SetUvTransform(const Transform& uvTransform) { uvTransform_ = uvTransform; }

	/// <summary>
	/// マテリアルデータのセッター
	/// </summary>
	void SetMaterialData(MaterialData* materialData) { materialData_ = materialData; }

	/// <summary>
	/// テクスチャのセッター
	/// </summary>
	void SetTexture(Texture* texture) { texture_ = texture; }


private:

	//テクスチャ
	Texture* texture_;

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	MaterialData* materialData_ = nullptr;

	//uvTransform
	Transform uvTransform_;

};

