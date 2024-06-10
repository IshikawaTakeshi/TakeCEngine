#pragma once
#include "../Vector3.h"
#include "../MyMath/Matrix4x4.h"
#include "../MyMath/VertexData.h"
#include "../MyMath/Transform.h"

#include <stdint.h>
#include <d3d12.h>
#include <wrl.h>

class Texture;
class DirectXCommon;
class Sphere {
public:
	Sphere() = default;
	~Sphere();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

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
	void InitializeCommandList(DirectXCommon* dxCommon, Texture* texture1, Texture* texture2);

private:

	//分割数
	static inline const uint32_t kSubdivision = 16;

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
	Matrix4x4* wvpData_ = nullptr;

	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Vector4* materialData_;

	//Texture
	Texture* Texture_ = nullptr;
	bool useMonsterBall = true;
};

