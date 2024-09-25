#pragma once

#include "../Include/ResourceDataStructure.h"
#include "../MyMath/Transform.h"
#include "../MyMath/TransformMatrix.h"
#include <d3d12.h>
#include <wrl.h>


class Mesh;
class Texture;
class DirectXCommon;
class ModelCommon;
class Model {
public:

	Model() = default;
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* ModelCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	//void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// objファイルを読む関数
	/// </summary>
	ModelData LoadObjFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);
	
	/// <summary>
	/// mtlファイルを読む関数
	/// </summary>
	ModelMaterialData LoadMtlFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);

public: //ゲッター

	Mesh* GetMesh() { return mesh_; }

private:

	ModelCommon* modelCommon_ = nullptr;

	//メッシュ
	Mesh* mesh_ = nullptr;

	//構築するModelData
	ModelData modelData_;
};

