#pragma once

#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "Animation/Animator.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "Mesh/Mesh.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "base/DirectXCommon.h"
#include "3d/ModelCommon.h"

#include <d3d12.h>
#include <wrl.h>
#include <memory>

//============================================================================
// Model class
//============================================================================
class Model {
public:

	Model() = default;
	~Model() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* ModelCommon,ModelData* modelData);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(Animation* animation, float animationTime);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// スカイボックス用描画処理
	/// </summary>
	void DrawSkyBox();

	/// <summary>
	/// スキニング計算
	/// </summary>
	/// <param name="skinningPso"></param>
	void Dispatch(PSO* skinningPso);

	/// <summary>
	/// パーティクル用描画処理
	/// </summary>
	/// <param name="instanceCount_"></param>
	void DrawForParticle(UINT instanceCount_);

	/// <summary>
	/// GPUパーティクル用描画処理
	/// </summary>
	void DrawForGPUParticle(UINT instanceCount);

public:

	//========================================================================
	// accessors
	//========================================================================
	
	//----- getter ---------------------------
	
	//メッシュの取得
	Mesh* GetMesh() { return mesh_.get(); }
	//スケルトンの取得
	Skeleton* GetSkeleton() { return skeleton_.get(); }
	//ModelDataの取得
	ModelData* GetModelData() { return modelData_; }
	//ModelCommonの取得
	ModelCommon* GetModelCommon() { return modelCommon_; }
	//テクスチャファイルパスの取得
	const std::string& GetTextureFilePath() const { return modelData_->material.textureFilePath; }
	//ローカル行列の取得
	const Matrix4x4& GetLocalMatrix() const { return localMatrix_; }

	//----- setter ---------------------------
	
	//メッシュの設定
	void SetMesh(Mesh* mesh) { mesh_.reset(mesh); }
	//ModelCommonの設定
	void SetModelCommon(ModelCommon* modelCommon) { modelCommon_ = modelCommon; }

private:

	//モデルの共通情報
	ModelCommon* modelCommon_ = nullptr;

	//メッシュ
	std::unique_ptr<Mesh> mesh_ = nullptr;
	//スケルトン
	std::unique_ptr<Skeleton> skeleton_;
	//スキンクラスター
	SkinCluster skinCluster_;

	//構築するModelData
	ModelData* modelData_;

	//移動・回転・拡縮情報
	Vector3 translate_;
	Quaternion rotate_;
	Vector3 scale_;
	Matrix4x4 localMatrix_;

	//UAVのインデックス
	uint32_t uavIndex_ = 0;
	//スケルトンを持っているかどうか
	bool haveSkeleton_ = true;
	//inputVertexのインデックス
	uint32_t inputIndex_ = 0;
};