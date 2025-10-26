#pragma once
#include "engine/math/Transform.h"
#include "engine/math/Matrix4x4.h"
#include "engine/math/Vector2.h"
#include "engine/camera/CameraForGPU.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>
#include <optional>
#include <numbers>

//============================================================================
//	Camera class
//============================================================================

class Camera {
public:

	enum class GameCameraState {
		FOLLOW,
		LOOKAT,
		ENEMY_DESTROYED,
	};

	Camera() = default;
	~Camera() = default;

	//初期化
	void Initialize(ID3D12Device* device);
	//更新処理
	void Update();
	//カメラシェイク
	void ShakeCamera();
	//ImGuiの更新処理
	void UpdateImGui();

public: //getter

	//cameraBufferの取得
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetCameraResource() const { return cameraResource_; }
	//カメラの状態の取得
	const GameCameraState& GetCameraState() const { return cameraState_; }
	
	//ワールド行列の取得
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	//ビュー行列の取得
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	//射影行列の取得
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	//ビュープロジェクション行列の取得
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	//回転行列の取得
	const Matrix4x4& GetRotationMatrix() const { return rotationMatrix_; }

	//カメラの回転の取得
	const Quaternion& GetRotate()const { return transform_.rotate; }
	//カメラの位置の取得
	const Vector3& GetTranslate() const { return transform_.translate; }
	//オフセットの取得
	const Vector3& GetOffset() const { return offset_; }
	
	//シェイクするかどうかの取得
	const bool& GetIsShaking() const { return isShaking_; }

public: //setter

	//カメラの状態リクエストの設定
	void SetCameraStateRequest(const GameCameraState state) { cameraStateRequest_ = state; }

	//カメラの回転の設定
	void SetRotate(const Quaternion& rotate);
	//カメラの位置、回転の設定
	void SetTranslate(const Vector3& Translate) { transform_.translate = Translate; }
	//オフセットの設定
	void SetOffset(const Vector3& offset) { offset_ = offset; }
	
	//オフセットの変化量の設定
	void SetFovX(const float fovX) { fovX_ = fovX; }
	//水平方向視野角の設定
	void SetAspectRatio(const float aspectRatio) { aspectRatio_ = aspectRatio; }
	//nearクリップ距離の設定
	void SetNearClip(const float nearClip) { nearClip_ = nearClip; }
	//farクリップ距離の設定
	void SetFarClip(const float farClip) { farClip_ = farClip; }
	//シェイクするかどうかの設定
	void SetIsShaking(const bool isShaking) { isShaking_ = isShaking; }
	//デバッグ状態かの設定
	void SetIsDebug(bool isDebug){ isDebug_ = isDebug; }
	//カメラシェイクの設定
	void SetShake(float duration, float range);
	
	//追従対象の位置の設定
	void SetFollowTargetPos(const Vector3& target) { followTargetPosition_ = target; }
	//追従対象の回転量の設定
	void SetFollowTargetRot(const Vector3& targetRot) { followTargetRotation_ = targetRot; }
	//補足対象の設定
	void SetFocusTargetPos(const Vector3& target) { focusTargetPosition_ = target; }
	//スティック情報の設定
	void SetStick(const Vector2& stick) { stick_ = stick; }

	//EnemyZoomの設定
	void SetEZoomEnemy(const bool isEZoomEnemy) { isEZoomEnemy_ = isEZoomEnemy; }
private:

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	CameraForGPU* cameraForGPU_;

	QuaternionTransform transform_;
	Vector3 nextPosition_;
	Vector3 direction_;
	Vector3 offset_;
	Vector3 offsetDelta_;
	Vector3 idealOffset_ = Vector3(0.0f, 0.0f, -50.0f); // 初期オフセット
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	//回転行列
	Matrix4x4 rotationMatrix_;

	//スティック情報
	Vector2 stick_;

	//カメラの状態リクエスト
	std::optional<GameCameraState> cameraStateRequest_ = std::nullopt;
	//カメラの状態
	GameCameraState cameraState_ = GameCameraState::FOLLOW;

	//追従対象
	Vector3 followTargetPosition_;
	Vector3 followTargetRotation_;
	float followSpeed_ = 0.3f; //追従速度
	float rotationSpeed_ = 0.1f; //回転追従速度

	//補足対象
	Vector3 focusTargetPosition_;
	
	//水平方向視野角
	float fovX_;
	//aspect比
	float aspectRatio_;
	//nearクリップ距離
	float nearClip_;
	//farクリップ距離
	float farClip_;
	float yawRot_ = 0.0f;
	float pitchRot_ = 0.0f;
	//デバッグ状態か
	bool isDebug_ = false;


	bool isShaking_ = false;     //シェイク中かどうか
	float shakeDuration_ = 0.0f; //シェイクの残り時間
	Vector3 originalPosition_;   //シェイク開始前のカメラ位置
	float shakeRange_ = 0.2f;    //シェイクの振幅

	//敵をズームしているか
	bool isEZoomEnemy_ = false;
	//ピッチの制限角度(70度)
	const float kPitchLimit = std::numbers::pi_v<float> / 180.0f * 70.0f; 


private:

	//デバッグカメラの更新
	void UpdateDebugCamera();
	//ゲームカメラの更新
	void UpdateGameCamera();

	//各カメラ状態の初期化
	void InitializeCameraFollow();
	void InitializeCameraLookAt();
	void InitializeCameraEnemyDestroyed();

	//各カメラ状態の更新
	void UpdateCameraFollow();
	void UpdateCameraLockOn();
	void UpdateCameraEnemyDestroyed();
};