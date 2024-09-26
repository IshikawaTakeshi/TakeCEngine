#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
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
	void Initialize(Object3dCommon* object3dCommon, Matrix4x4 cameraView,const std::string& filePath);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update(int num);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

public: //getter

	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotation() const { return transform_.rotate; }
	Vector3 GetPosition() const { return transform_.translate; }

public: //setter

	void SetModel(const std::string& filePath);
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

