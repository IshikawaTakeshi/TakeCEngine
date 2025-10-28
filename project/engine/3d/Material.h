#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cstdint>

//マテリアル情報構造体
struct MaterialData {
	Vector4 color; //カラー
	Matrix4x4 uvTransform; //UVトランスフォーム
	uint32_t enableLighting; //ライティングを有効にするフラグ
	float shininess; //鏡面反射の強さ
	float envCoefficient; //環境光の強さ
};

//前方宣言
class DirectXCommon;

//============================================================================
// Material class
//============================================================================
class Material {
public:

	Material() = default;
	~Material() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const std::string& filePath, const std::string& envMapfilePath);

	void Update();

	/// <summary>
	/// 更新処理
	/// </summary>
	void UpdateMaterialImGui();

	/// <summary>
	/// マテリアルリソース初期化
	/// </summary>
	void InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device);

public:

	//========================================================================
	// accessors
	//========================================================================

	//----- getter ---------------------------

	// マテリアルリソースの取得
	ID3D12Resource* GetMaterialResource() const { return materialResource_.Get(); }
	// materialDataの取得
	MaterialData* GetMaterialData() const { return materialData_; }
	// テクスチャファイルパスの取得
	const std::string& GetTextureFilePath() const { return textureFilePath_; }
	// 環境マップ用テクスチャファイルパスの取得
	const std::string& GetEnvMapFilePath() const { return envMapFilePath_; }


	//----- setter ---------------------------


	// uvTransformの設定
	void SetUvTransform(const EulerTransform& uvTransform) { uvTransform_ = uvTransform; }
	// スケールの設定
	void SetUvScale(Vector3 scale) { uvTransform_.scale = scale; }
	// 回転の設定
	void SetUvRotate(Vector3 rotate) { uvTransform_.rotate = rotate; }
	// 平行移動の設定
	void SetUvTranslate(Vector3 translate) { uvTransform_.translate = translate; }
	// materialDataの設定
	void SetMaterialData(MaterialData* materialData) { materialData_ = materialData; }
	// ライティング有効フラグの設定
	void SetEnableLighting(bool enable) { materialData_->enableLighting = enable; }
	// マテリアルカラーの設定
	void SetMaterialColor(Vector4 color) { materialData_->color = color; }
	// アルファ値の設定
	void SetAlpha(float alpha) { materialData_->color.w = alpha; }
	// 鏡面反射の強さの設定
	void SetShininess(float shininess) { materialData_->shininess = shininess; }
	// 環境光の強さの設定
	void SetEnvCoefficient(float envCoefficient) { materialData_->envCoefficient = envCoefficient; }
	// テクスチャファイルパスの設定
	void SetTextureFilePath(const std::string& filePath) { textureFilePath_ = filePath; }
	// 環境マップ用テクスチャファイルパスの設定
	void SetEnvMapFilePath(const std::string& filePath) { envMapFilePath_ = filePath; }

private:

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	MaterialData* materialData_ = nullptr;

	//uvTransform
	EulerTransform uvTransform_;

	//テクスチャファイルパス
	std::string textureFilePath_;
	//環境マップ用テクスチャファイルパス
	std::string envMapFilePath_;
};