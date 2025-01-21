#pragma once

#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "Animation/Animator.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "Mesh/Mesh.h"



#include <d3d12.h>
#include <wrl.h>
#include <memory>


class Texture;
class SrvManager;
class DirectXCommon;
class ModelCommon;
class Model {
public:

	Model() = default;
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* ModelCommon,ModelData& modelData, const std::string& modelDirectoryPath, const std::string& filename);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	//void UpdateSkeleton();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void DrawSkyBox();

	void DrawForASkinningModel();

	//void DisPatchForASkinningModel();


	/// <summary>
	/// パーティクル描画
	/// </summary>
	/// <param name="instanceCount_"></param>
	void DrawForParticle(UINT instanceCount_);

	/// <summary>
	/// アニメーションの適用
	/// </summary>
	//void ApplyAnimation();


public: //ゲッター

	//メッシュの取得
	Mesh* GetMesh() { return mesh_.get(); }

	//スケルトンの取得
	Skeleton* GetSkeleton() { return skeleton_.get(); }

	//ModelDataの取得
	ModelData& GetModelData() { return modelData_; }

	//ModelCommonの取得
	ModelCommon* GetModelCommon() { return modelCommon_; }

	//Animationの取得
	Animation& GetAnimation() { return animation_; }


	//テクスチャファイルパスの取得
	const std::string& GetTextureFilePath() const { return modelData_.material.textureFilePath; }

	//ローカル行列の取得
	const Matrix4x4& GetLocalMatrix() const { return localMatrix_; }

public: //セッター

	//メッシュの設定
	void SetMesh(Mesh* mesh) { mesh_.reset(mesh); }

	//ModelCommonの設定
	void SetModelCommon(ModelCommon* modelCommon) { modelCommon_ = modelCommon; }

private:

	ModelCommon* modelCommon_ = nullptr;

	//メッシュ
	std::unique_ptr<Mesh> mesh_ = nullptr;
	//スケルトン
	std::unique_ptr<Skeleton> skeleton_;
	//スキンクラスター
	SkinCluster skinCluster_;

	//構築するModelData
	ModelData modelData_;
	Animation animation_;

	Vector3 translate_;
	Quaternion rotate_;
	Vector3 scale_;
	Matrix4x4 localMatrix_;

	//再生中の時刻
	float animationTime = 0.0f;

	uint32_t srvIndex_ = 0;
	uint32_t uavIndex_ = 0;
	uint32_t skinnedsrvIndex_ = 0;
};