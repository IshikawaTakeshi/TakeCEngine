#pragma once
#include "Include/ResourceDataStructure.h"
#include "MyMath/Transform.h"
#include "MyMath/TransformMatrix.h"
#include <d3d12.h>
#include <wrl.h>

class Model;
class Object3dCommon;
class Object3d {

public: // publicメンバ関数

	Object3d() = default;
	~Object3d();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, Matrix4x4 cameraView,Model* model);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update(int num);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// objファイル読み取り
	/// </summary>
	ModelData LoadObjFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);

	/// <summary>
	/// mtlファイル読み取り
	/// </summary>
	ModelMaterialData LoadMtlFile(const std::string& resourceDirectoryPath, const std::string& modelDirectoryPath, const std::string& filename);

public: //getter

	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotation() const { return transform_.rotate; }
	Vector3 GetPosition() const { return transform_.translate; }

public: //setter

	void SetModel(Model* model) { model_ = model; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
	void SetPosition(const Vector3& position) { transform_.translate = position; }


private: // privateメンバ変数

	Object3dCommon* object3dCommon_ = nullptr;

	//モデル
	Model* model_ = nullptr;

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

