#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "ResourceDataStructure.h"
#include "camera/Camera.h"
#include "3d/Model.h"
#include "Animation/Animator.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "PSOType.h"

//前方宣言
class Object3dCommon;

//============================================================================
// Object3d class
//============================================================================
class Object3d {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//========================================================================
	// functions
	//========================================================================

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

	void UpdateImGui([[maybe_unused]]const std::string& name);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// スキニング計算
	/// </summary>
	void Dispatch();

public: 
	//========================================================================
	// accessors
	//========================================================================

	//----- getter ---------------------------

	//Transformの取得
	const EulerTransform& GetTransform() const { return transform_; }
	//ワールド行列の取得
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	//WVP行列の取得
	const Vector3& GetScale() const { return transform_.scale; }
	//回転量の取得
	const Vector3& GetRotate() const { return transform_.rotate; }
	//位置の取得
	const Vector3& GetTranslate() const { return transform_.translate; }
	//モデルの中心位置の取得
	const Vector3& GetCenterPosition() const;
	//モデルの取得
	Model* GetModel() { return model_.get(); }

	//Animationの取得
	Animation* GetAnimation() { return animation_; }
	//アニメーションの尺の取得
	float GetDuration() { return animation_->duration; }
	//アニメーションの再生時間の取得
	float GetAnimationTime() { return animationTime_; }

	//----- setter ---------------------------

	//カメラの設定
	void SetCamera(Camera* camera) { camera_ = camera; }
	//Transformの設定
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotation) { transform_.rotate = rotation; }
	void SetTranslate(const Vector3& position) { transform_.translate = position; }

	//ワールド行列の設定
	void SetWorldMatrix(const Matrix4x4& worldMatrix);
	//アニメーションの設定
	void SetAnimation(Animation* animation);
	//ペアレントの設定
	void SetParent(const Matrix4x4& parent) { parentWorldMatrix_ = &parent; }
	//モデルファイルパスの設定
	void SetModelFilePath(const std::string& filePath) { modelFilePath_ = filePath; }

private:

	//アニメーション処理
	void AnimationUpdate();

protected: // privateメンバ変数

	//Object3d共通情報
	Object3dCommon* object3dCommon_ = nullptr;

	//ペアレント
	const Matrix4x4* parentWorldMatrix_ = nullptr;

	//モデル
	std::unique_ptr<Model> model_ = nullptr;
	//モデルファイルパス
	std::string modelFilePath_;

	//アニメーション
	Animation* animation_;
	float animationTime_ = 0.0f;
	bool isAnimation_ = true;

	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	Vector3 worldPosition_;
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;

};