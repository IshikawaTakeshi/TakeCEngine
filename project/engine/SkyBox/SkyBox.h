#pragma once
#include "engine/base/DirectXCommon.h"
#include "engine/base/ComPtrAliasTemplates.h"
#include "engine/math/TransformMatrix.h"
#include "engine/3d/Model.h"

//前方宣言
class Camera;
class PSO;

//===========================================================================
// SkyBox class
//===========================================================================
class SkyBox {
public:

	//=======================================================================
	// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SkyBox() = default;
	~SkyBox() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon,const std::string& texturefilePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void SetMaterialColor(const Vector4& color);

private: // privateメンバ変数

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;
	//PSO
	std::unique_ptr<PSO> pso_ = nullptr;
	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;


	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//プリミティブのハンドル
	uint32_t primitiveHandle_ = 0;

	//Transform
	EulerTransform transform_{};
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;
};