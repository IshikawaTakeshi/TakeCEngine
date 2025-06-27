#pragma once

#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "Animation/Animator.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "3d/Mesh/Mesh.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "base/DirectXCommon.h"
#include "3d/ModelCommon.h"
#include "Animation/VertexInfluence.h"

#include <d3d12.h>
#include <wrl.h>
#include <memory>


//モデル1個分のデータ
struct ModelData {
	std::string fileName; //モデル名
	std::map<std::string, JointWeightData> skinClusterData;
	std::vector<VertexInfluence> influences;
	SkinningInfo skinningInfoData;
	ModelMesh mesh; //メッシュデータ
	Node rootNode;
	bool haveBone = false;

	std::unique_ptr<ModelData> Clone() const {
		auto clonedData = std::make_unique<ModelData>();
		clonedData->fileName = fileName;
		clonedData->skinClusterData = skinClusterData;
		clonedData->skinningInfoData = skinningInfoData;
		clonedData->mesh = mesh; // ModelMeshはコピー可能
		clonedData->rootNode = rootNode; // Nodeもコピー可能
		clonedData->haveBone = haveBone;
		return clonedData;
	}
};

class Model {
public:

	Model();
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* ModelCommon,std::unique_ptr<ModelData> modelData);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(Animation* animation, float animationTime);

	//void UpdateSkeleton();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(PSO* graphicPso);
	void DisPatch(PSO* skinningPso);

	/// <summary>
	/// パーティクル描画
	/// </summary>
	/// <param name="instanceCount_"></param>
	void DrawForParticle(PSO* graphicPso,UINT instanceCount_);

	void DrawForGPUParticle(PSO* graphicPso,UINT instanceCount);

	std::unique_ptr<Model> Clone() const;


public: //ゲッター

	//スケルトンの取得
	Skeleton* GetSkeleton() { return skeleton_.get(); }

	//ModelDataの取得
	ModelData* GetModelData() { return modelData_.get(); }

	//ModelCommonの取得
	ModelCommon* GetModelCommon() { return modelCommon_; }

	//テクスチャファイルパスの取得
	const std::string& GetTextureFilePath(const uint32_t& materialNum) const;

	//ローカル行列の取得
	const Matrix4x4& GetLocalMatrix() const { return localMatrix_; }

public: //セッター

	//ModelCommonの設定
	void SetModelCommon(ModelCommon* modelCommon) { modelCommon_ = modelCommon; }

private:

	ModelCommon* modelCommon_ = nullptr;

	//スケルトン
	std::unique_ptr<Skeleton> skeleton_;
	//スキンクラスター
	SkinCluster skinCluster_;

	//構築するModelData
	std::unique_ptr<ModelData> modelData_;

	Vector3 translate_;
	Quaternion rotate_;
	Vector3 scale_;
	Matrix4x4 localMatrix_;

	uint32_t uavIndex_ = 0;

	bool haveSkeleton_ = true;

	uint32_t inputIndex_ = 0;
};