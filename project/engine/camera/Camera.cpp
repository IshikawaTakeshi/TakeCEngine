#include "Camera.h"
#include "base/WinApp.h"
#include "base/DirectXCommon.h"
#include "base/ImGuiManager.h"
#include "base/TakeCFrameWork.h"
#include "io/Input.h"
#include "Collision/CollisionManager.h"
#include "math/MatrixMath.h"
#include "math/Easing.h"
#include "math/Vector3Math.h"
#include "math/MathEnv.h"


//=============================================================================
// 初期化
//=============================================================================

void Camera::Initialize(ID3D12Device* device,const std::string& configData) {
	
	//カメラの各種パラメータ初期化
	cameraConfig_ = TakeCFrameWork::GetJsonLoader()->LoadJsonData<CameraConfig>(configData);
	worldMatrix_ = MatrixMath::MakeAffineMatrix(cameraConfig_.transform_);
	viewMatrix_ = MatrixMath::Inverse(worldMatrix_);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		cameraConfig_.fovX_, 
		cameraConfig_.aspectRatio_,
		cameraConfig_.nearClip_,
		cameraConfig_.farClip_);
	viewProjectionMatrix_ = MatrixMath::Multiply(viewMatrix_, projectionMatrix_);
	rotationMatrix_ = MatrixMath::MakeIdentity4x4();

	followSpeed_ = 0.3f;
	rotationSpeed_ = 0.1f;

	//カメラ用バッファリソース生成
	cameraResource_ = DirectXCommon::CreateBufferResource(device, sizeof(CameraForGPU));
	cameraResource_->SetName(L"Camera::cameraResource_");
	cameraForGPU_ = nullptr;
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
	cameraForGPU_->worldPosition = cameraConfig_.transform_.translate;
}

//=============================================================================
// 更新
//=============================================================================

void Camera::Update() {
#ifdef _DEBUG
	//デバッグカメラとゲームカメラの切り替え
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		isDebug_ = !isDebug_;
	}
#endif // _DEBUG

	if (isDebug_) {
		//デバッグカメラの更新
		UpdateDebugCamera();
		
	} else {
		//ゲームカメラの更新
		UpdateGameCamera();
	}
	
	// クォータニオンを正規化して数値誤差を防ぐ
	cameraConfig_.transform_.rotate = QuaternionMath::Normalize(cameraConfig_.transform_.rotate);

	//各種行列の計算
	rotationMatrix_ = MatrixMath::MakeRotateMatrix(cameraConfig_.transform_.rotate);
	worldMatrix_ = MatrixMath::MakeAffineMatrix(cameraConfig_.transform_);
	viewMatrix_ = MatrixMath::Inverse(worldMatrix_);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		cameraConfig_.fovX_,
		cameraConfig_.aspectRatio_,
		cameraConfig_.nearClip_,
		cameraConfig_.farClip_
	);
	viewProjectionMatrix_ = MatrixMath::Multiply(viewMatrix_, projectionMatrix_);

	//GPUに転送するパラメータの更新
	cameraForGPU_->worldPosition = cameraConfig_.transform_.translate;
	cameraForGPU_->ProjectionInverse = MatrixMath::Inverse(projectionMatrix_);
}

//=============================================================================
// シェイクの設定
//=============================================================================

void Camera::SetShake(float duration, float range) {
	if (!isShaking_) {
		isShaking_ = true;
		shakeDuration_ = duration;                // シェイク継続時間を設定
		shakeRange_ = range;                      // シェイクの振幅を設定
		originalPosition_ = cameraConfig_.offsetDelta_; // 元の位置を保存
	}
}

//=============================================================================
// カメラシェイクの更新
//=============================================================================

void Camera::ShakeCamera() {
	if (isShaking_) {
		if (shakeDuration_ > 0.0f) {
			// ランダムな揺れを計算
			float offsetX = (static_cast<float>(rand()) / RAND_MAX) * shakeRange_ * 2.0f - shakeRange_;
			float offsetY = (static_cast<float>(rand()) / RAND_MAX) * shakeRange_ * 2.0f - shakeRange_;
			float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * shakeRange_ * 2.0f - shakeRange_;

			// カメラの位置を揺らす
			cameraConfig_.offsetDelta_ = originalPosition_ + Vector3{ offsetX, offsetY, offsetZ };

			// 残り時間を減らす
			shakeDuration_ -= 1.0f;
		} else {
			// シェイク終了
			cameraConfig_.offsetDelta_ = originalPosition_; // 元の位置に戻す
			isShaking_ = false;
		}
	}
}

//=============================================================================
// ImGuiによるパラメータ調整
//=============================================================================

void Camera::UpdateImGui() {
#ifdef _DEBUG
	ImGui::DragFloat3("Translate", &cameraConfig_.transform_.translate.x, 0.01f);
	ImGui::DragFloat4("Rotate", &cameraConfig_.transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("offset", &cameraConfig_.offset_.x, 0.01f);
	ImGui::DragFloat3("offsetDelta", &cameraConfig_.offsetDelta_.x, 0.01f);
	ImGui::DragFloat("yawRot", &yawRot_, 0.01f);
	ImGui::DragFloat("pitchRot", &pitchRot_, 0.01f);
	ImGui::DragFloat("FovX", &cameraConfig_.fovX_, 0.01f);
	ImGui::DragFloat("followSpeed", &followSpeed_, 0.01f);
	ImGui::DragFloat("rotationSpeed", &rotationSpeed_, 0.01f);
	ImGui::Checkbox("isDebug", &isDebug_);

	// カメラデータ保存ボタン
	if (ImGui::Button("Save Camera Data"))
	{
		ImGui::OpenPopup("Save Camera");
	}

	// 保存用ポップアップ
	if (ImGui::BeginPopupModal("Save Camera", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char filenameBuf[256] = "camera.json";
		ImGui::Text("Input filename for camera data:");
		ImGui::InputText("Filename", filenameBuf, sizeof(filenameBuf));

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			//Jsonファイルの名前を入力して保存
			cameraConfig_.filePath = std::string(filenameBuf);
			TakeCFrameWork::GetJsonLoader()->SaveJsonData<CameraConfig>(cameraConfig_.filePath, cameraConfig_);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif // DEBUG
}



//=============================================================================
// デバッグカメラの更新
//=============================================================================

void Camera::UpdateDebugCamera() {

	// クォータニオンで回転を管理
	Quaternion rotationDelta = QuaternionMath::IdentityQuaternion();

	if (Input::GetInstance()->IsPressMouse(1)) {
		// マウス入力による回転計算
		float deltaPitch = (float)Input::GetInstance()->GetMouseMove().lY * 0.001f; // X軸回転
		float deltaYaw = (float)Input::GetInstance()->GetMouseMove().lX * 0.001f;   // Y軸回転

		// クォータニオンを用いた回転計算
		Quaternion yawRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
			Vector3(0, 1, 0), deltaYaw);
		Quaternion pitchRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
			QuaternionMath::RotateVector(Vector3(1, 0, 0), yawRotation * cameraConfig_.transform_.rotate), deltaPitch);

		//回転の補間
		rotationDelta = pitchRotation * yawRotation;
	}

	// 累積回転を更新
	cameraConfig_.transform_.rotate = rotationDelta * cameraConfig_.transform_.rotate;

	if (Input::GetInstance()->IsPressMouse(2)) {
		cameraConfig_.offsetDelta_.x += (float)Input::GetInstance()->GetMouseMove().lX * 0.1f;
		cameraConfig_.offsetDelta_.y -= (float)Input::GetInstance()->GetMouseMove().lY * 0.1f;
	}

	// オフセットを考慮したワールド行列の計算
	cameraConfig_.offsetDelta_.z += (float)Input::GetInstance()->GetWheel() * 0.1f;

	// 回転を適用
	cameraConfig_.offset_ = cameraConfig_.offsetDelta_;
	cameraConfig_.offset_ = QuaternionMath::RotateVector(cameraConfig_.offset_, cameraConfig_.transform_.rotate);
	cameraConfig_.transform_.translate = cameraConfig_.offset_;

}

//=============================================================================
// ゲームカメラの更新
//=============================================================================

void Camera::UpdateGameCamera() {
	
	if (cameraStateRequest_) {
		cameraState_ = cameraStateRequest_.value();

		switch (cameraState_) {
		case Camera::GameCameraState::FOLLOW:
			InitializeCameraFollow();
			break;
		case Camera::GameCameraState::LOOKAT:
			InitializeCameraLookAt();
			break;
		case Camera::GameCameraState::ENEMY_DESTROYED:
			InitializeCameraEnemyDestroyed();
			break;
		default:
			break;
		}

		cameraStateRequest_ = std::nullopt;
	}

	switch (cameraState_) {
	case Camera::GameCameraState::FOLLOW:
		UpdateCameraFollow();
		break;
	case Camera::GameCameraState::LOOKAT:
		UpdateCameraLockOn();
		break;
	case Camera::GameCameraState::ENEMY_DESTROYED:
		UpdateCameraEnemyDestroyed();
		break;
	default:

		nextPosition_ = cameraConfig_.transform_.translate; // デフォルトの位置を保持
		break;
	}

	
	
}

#pragma region Follow State
//=============================================================================
// Follow状態の処理
//=============================================================================

void Camera::InitializeCameraFollow() {

	followSpeed_ = 0.3f;
	cameraConfig_.offset_ = cameraConfig_.offsetDelta_;
	cameraConfig_.offsetDelta_ = Vector3(0.0f, 5.0f, -50.0f);
}


void Camera::UpdateCameraFollow() {

	// クォータニオンで回転を管理
	Quaternion rotationDelta = QuaternionMath::IdentityQuaternion();

	//スティックによる回転計算
	float deltaPitch = stick_.y * 0.02f; // X軸回転
	float deltaYaw = stick_.x * 0.02f;   // Y軸回転

	yawRot_ += deltaYaw;
	pitchRot_ += deltaPitch;

	pitchRot_ = std::clamp(pitchRot_, -kPitchLimit,kPitchLimit);

	// クォータニオンを用いた回転計算
	Quaternion yawRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
		Vector3(0, 1, 0), yawRot_);
	Quaternion pitchRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
		QuaternionMath::RotateVector(Vector3(1, 0, 0), yawRotation), pitchRot_);

	//回転の合成
	rotationDelta = pitchRotation * yawRotation;

	// オフセットに回転を適用
	cameraConfig_.offset_ = QuaternionMath::RotateVector(cameraConfig_.offsetDelta_, rotationDelta);
	//カメラ位置の計算
	Vector3 targetPosition_ = followTargetPosition_ + cameraConfig_.offset_;
	nextPosition_ = Easing::Lerp(cameraConfig_.transform_.translate, targetPosition_, followSpeed_);

	//埋まり回避
	direction_ = Vector3Math::Normalize(nextPosition_ - followTargetPosition_);
	float distance = Vector3Math::Length(nextPosition_ - followTargetPosition_);

	Ray ray;
	ray.origin = followTargetPosition_;
	ray.direction = direction_;
	ray.distance = distance;
	RayCastHit hitInfo;

	//コライダーのマスク
	uint32_t layerMask = ~static_cast<uint32_t>(CollisionLayer::Ignoe);
	if (CollisionManager::GetInstance()->RayCast(ray, hitInfo,layerMask)) {
		// 衝突した場合は、ヒット位置の少し手前にカメラを配置するなど
		float margin = 0.1f; // 衝突位置から少し手前に移動するマージン
		cameraConfig_.transform_.translate = hitInfo.position - direction_ * margin;
	} else {
		// 衝突しなかった場合は、通常の位置に移動
		cameraConfig_.transform_.translate = nextPosition_;
	}
	cameraConfig_.transform_.rotate = Easing::Slerp(cameraConfig_.transform_.rotate, rotationDelta, rotationSpeed_);

	//Rスティック押し込みでカメラの状態変更
	if (Input::GetInstance()->TriggerButton(0,GamepadButtonType::RightStick)) {
		cameraStateRequest_ = GameCameraState::LOOKAT;
	}
}

#pragma endregion

#pragma region LookAt State

//=============================================================================
// LookAt状態の処理
//=============================================================================

void Camera::InitializeCameraLookAt() {

	followSpeed_ = 0.4f;
	cameraConfig_.offsetDelta_ = Vector3(0.0f, 5.0f, -50.0f);
}

void Camera::UpdateCameraLockOn() {

	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(focusTargetPosition_ - cameraConfig_.transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	// 高さを持った固定オフセット
	cameraConfig_.offset_ = cameraConfig_.offsetDelta_;

	// ターゲットからの相対位置に補間移動
	Vector3 desiredPosition = followTargetPosition_ + QuaternionMath::RotateVector(cameraConfig_.offset_, cameraConfig_.transform_.rotate);
	nextPosition_ = Easing::Lerp(cameraConfig_.transform_.translate, desiredPosition, followSpeed_);

	//埋まり回避
	direction_ = Vector3Math::Normalize(nextPosition_ - followTargetPosition_);
	float distance = Vector3Math::Length(nextPosition_ - followTargetPosition_);

	Ray ray;
	ray.origin = followTargetPosition_;
	ray.direction = direction_;
	ray.distance = distance;
	RayCastHit hitInfo;

	//コライダーのマスク
	uint32_t layerMask = ~static_cast<uint32_t>(CollisionLayer::Ignoe);
	if (CollisionManager::GetInstance()->RayCast(ray, hitInfo,layerMask)) {
		// 衝突した場合は、ヒット位置の少し手前にカメラを配置するなど
		float margin = 0.1f; // 衝突位置から少し手前に移動するマージン
		cameraConfig_.transform_.translate = hitInfo.position - direction_ * margin;
	} else {
		// 衝突しなかった場合は、通常の位置に移動
		cameraConfig_.transform_.translate = nextPosition_;
	}

	// 回転補間
	cameraConfig_.transform_.rotate = Easing::Slerp(cameraConfig_.transform_.rotate, targetRotation, rotationSpeed_);

	// 状態切り替え
	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::RightStick)) {
		// transform_.rotateからforwardベクトルを算出
		Vector3 forward = QuaternionMath::RotateVector(Vector3(0,0,1), cameraConfig_.transform_.rotate);
		//yaw
		yawRot_ = std::atan2(forward.x, forward.z);
		//pitch
		pitchRot_ = std::asin(-forward.y);
		//必要ならclampする
		pitchRot_ = std::clamp(pitchRot_, -kPitchLimit, kPitchLimit);

		//状態遷移リクエスト(FOLLOW)
		cameraStateRequest_ = GameCameraState::FOLLOW;
	}
}
#pragma endregion

#pragma region EnemyDestroyed State

//=============================================================================
// EnemyDestroyed状態の処理
//=============================================================================

void Camera::InitializeCameraEnemyDestroyed() {

	followSpeed_ = 0.1f;
	cameraConfig_.offsetDelta_ = Vector3(0.0f, 5.0f, -30.0f);
	isEZoomEnemy_ = true;
}

void Camera::UpdateCameraEnemyDestroyed() {

	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(focusTargetPosition_ - cameraConfig_.transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	// 高さを持った固定オフセット
	cameraConfig_.offset_ = cameraConfig_.offsetDelta_;

	// ターゲットからの相対位置に補間移動
	Vector3 desiredPosition = followTargetPosition_ + QuaternionMath::RotateVector(cameraConfig_.offset_, cameraConfig_.transform_.rotate);
	nextPosition_ = Easing::Lerp(cameraConfig_.transform_.translate, desiredPosition, followSpeed_);

	//埋まり回避
	direction_ = Vector3Math::Normalize(nextPosition_ - followTargetPosition_);
	float distance = Vector3Math::Length(nextPosition_ - followTargetPosition_);

	Ray ray;
	ray.origin = followTargetPosition_;
	ray.direction = direction_;
	ray.distance = distance;
	RayCastHit hitInfo;

	//コライダーのマスク
	uint32_t layerMask = ~static_cast<uint32_t>(CollisionLayer::Ignoe);
	if (CollisionManager::GetInstance()->RayCast(ray, hitInfo,layerMask)) {
		// 衝突した場合は、ヒット位置の少し手前にカメラを配置するなど
		float margin = 0.1f; // 衝突位置から少し手前に移動するマージン
		cameraConfig_.transform_.translate = hitInfo.position - direction_ * margin;
	} else {
		// 衝突しなかった場合は、通常の位置に移動
		cameraConfig_.transform_.translate = nextPosition_;
	}

	// 回転補間
	cameraConfig_.transform_.rotate = Easing::Slerp(cameraConfig_.transform_.rotate, targetRotation, followSpeed_);
	// transform_.rotateからforwardベクトルを算出
	Vector3 forward = QuaternionMath::RotateVector(Vector3(0,0,1), cameraConfig_.transform_.rotate);
	//yaw
	yawRot_ = std::atan2(forward.x, forward.z);
	//pitch
	pitchRot_ = std::asin(-forward.y);
	//必要ならclampする
	pitchRot_ = std::clamp(pitchRot_, -kPitchLimit, kPitchLimit);

	// 状態切り替え
	if (isEZoomEnemy_ == false) {

		//状態遷移リクエスト(FOLLOW)
		cameraStateRequest_ = GameCameraState::FOLLOW;
	}
}

#pragma endregion

//=============================================================================
// 回転の設定
//=============================================================================

void Camera::SetRotate(const Quaternion& rotate) {
	cameraConfig_.transform_.rotate = rotate;
}

//=============================================================================
// 上方向ベクトルの取得
//=============================================================================
const Vector3& Camera::GetUpVector() const {

	static Vector3 upVector = QuaternionMath::RotateVector(Vector3(0, 1, 0), cameraConfig_.transform_.rotate);
	return upVector;
}