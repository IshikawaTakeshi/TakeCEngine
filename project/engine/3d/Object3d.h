#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "ResourceDataStructure.h"

#include "Transform.h"
#include "TransformMatrix.h"
#include "PSOType.h"

class Model;
class Camera;
class Object3dCommon;
class Object3d {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	Object3d() = default;
	~Object3d();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update();

	#ifdef _DEBUG
	void UpdateImGui(int id);
	#endif // _DEBUG

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	void DrawForSkinningModel();

	void DisPatchForASkinningModel();

public: //getter

	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotation() const { return transform_.rotate; }
	Vector3 GetPosition() const { return transform_.translate; }

public: //setter

	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
	void SetPosition(const Vector3& position) { transform_.translate = position; }


protected: // privateメンバ変数

	Object3dCommon* object3dCommon_ = nullptr;

	//モデル
	Model* model_ = nullptr;

	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;

};