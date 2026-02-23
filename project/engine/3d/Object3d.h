#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>

#include "engine/camera/Camera.h"
#include "engine/3d/Model.h"
#include "engine/3d/Light/LightCameraInfo.h"
#include "engine/Animation/Animator.h"
#include "engine/math/Transform.h"
#include "engine/math/TransformMatrix.h"
#include "engine/base/PSOType.h"

//前方宣言
class Object3dCommon;

//============================================================================
// Object3d class
//============================================================================
class Object3d {
public:

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

	void DrawShadow(const LightCameraInfo& lightCamera);

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
	const Matrix4x4& GetWorldMatrix() const { 
		return worldMatrix_;
 }
	//WVP行列の取得
	const Vector3& GetScale() const { return transform_.scale; }
	//回転量の取得
	const Vector3& GetRotate() const { return transform_.rotate; }
	//位置の取得
	const Vector3& GetTranslate() const { return transform_.translate; }

	const Vector3& GetWorldPosition() const { return worldPosition_; }
	//モデルの中心位置の取得
	const Vector3& GetCenterPosition() const;
	//モデルの取得
	Model* GetModel() { return model_.get(); }

	//Animationの取得
	Animation* GetAnimation() { return animation_.get(); }
	//アニメーションの尺の取得
	float GetDuration() { return animation_->duration; }
	//アニメーションの再生時間の取得
	float GetAnimationTime() { return animationTime_; }
	//モデルファイルパスの取得
	const std::string& GetModelFilePath() const { return modelFilePath_; }
	//外部からアニメーションを設定するかどうかの取得
	bool GetUseExternalAnimation() const { return useExternalAnimation_; }

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
	//外部からアニメーションを設定するかどうかの設定
	void SetUseExternalAnimation(bool useExternal) { useExternalAnimation_ = useExternal; }

private:

	//アニメーション処理
	void AnimationUpdate();

protected: // privateメンバ変数

	//Object3d共通情報
	Object3dCommon* object3dCommon_ = nullptr;

	//ペアレント
	const Matrix4x4* parentWorldMatrix_ = nullptr;

	//モデル
	std::shared_ptr<Model> model_;
	//モデルファイルパス
	std::string modelFilePath_;

	//アニメーション
	std::unique_ptr<Animation> animation_;
	float animationTime_ = 0.0f;
	bool isAnimation_ = true;
	//外部からアニメーションを設定するかどうか
	bool useExternalAnimation_ = false;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	ComPtr<ID3D12Resource> shadowWvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* transformMatrixData_ = nullptr;
	TransformMatrix* shadowTransformMatrixData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	Vector3 worldPosition_;
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 shadowWVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;

};