#pragma once

#include "../Include/ResourceDataStructure.h"
#include "../MyMath/Transform.h"
#include "../MyMath/TransformMatrix.h"
#include <d3d12.h>
#include <wrl.h>

class Mesh;
class Texture;
class DirectXCommon;
class Model {
public:

	Model() = default;
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView,
		const std::string& resourceDirectoryPath,
		const std::string& modelDirectoryPath, 
		const std::string& filename);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void DrawCall(DirectXCommon* dxCommon);

	/// <summary>
	/// directionalLightData初期化
	/// </summary>
	void InitializeDirectionalLightData(DirectXCommon* dxCommon);


	/// <summary>
	/// objファイルを読む関数
	/// </summary>
	ModelData LoadObjFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);
	
	/// <summary>
	/// mtlファイルを読む関数
	/// </summary>
	ModelMaterialData LoadMtlFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);

public: //ゲッター



public: //セッター

	void SetTransform(const Transform& transform) { transform_ = transform; }

	void SetWorldMatrix(const Matrix4x4& worldMatrix) { worldMatrix_ = worldMatrix; }

	void SetTextureFilePath(const std::string& textureFilePath) { modelData_.material.textureFilePath = textureFilePath; }

private:

	//メッシュ
	Mesh* mesh_;

	//構築するModelData
	ModelData modelData_;

	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* transformMatrixData_ = nullptr;

	//平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLightData* directionalLightData_ = nullptr;;

	//CPU用のTransform
	Transform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;
};

