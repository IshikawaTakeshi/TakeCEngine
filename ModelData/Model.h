#pragma once

#include "../Vector2.h"
#include "../Vector3.h"
#include "../Vector4.h"
#include "../MyMath/Transform.h"
#include "../MyMath/Matrix4x4.h"
#include "../MyMath/TransformMatrix.h"
#include "../Include/ResourceDataStructure.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>

struct ModelMaterialData {

	std::string textureFilePath;
};

struct ModelData {

	std::vector<VertexData> vertices;
	ModelMaterialData material;
};


class Texture;
class DirectXCommon;
class Model {
public:

	Model() = default;
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView, const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void DrawCall(DirectXCommon* dxCommon, Texture* texture);

	/// <summary>
	/// VertexData初期化
	/// </summary>
	void InitializeVertexData(DirectXCommon* dxCommon);

	/// <summary>
	/// MaterialData初期化
	/// </summary>
	void InitializeMaterialData(DirectXCommon* dxCommon);

	/// <summary>
	/// directionalLightData初期化
	/// </summary>
	void InitializeDirectionalLightData(DirectXCommon* dxCommon);


	/// <summary>
	/// objファイルを読む関数
	/// </summary>
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	
	/// <summary>
	/// mtlファイルを読む関数
	/// </summary>
	ModelMaterialData LoadMtlFile(const std::string& directoryPath, const std::string& filename);



private:

	ModelData modelData_; //構築するModelData

	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	//頂点データ
	VertexData* vertexData_ = nullptr;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* transformMatrixData_ = nullptr;

	//平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLightData* directionalLightData_ = nullptr;;

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	MaterialData* materialData_ = nullptr;


	//CPU用のTransform
	Transform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;
};

