#include "BehaviorRunning.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Quaternion.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　コンストラクタ
//===================================================================================
BehaviorRunning::BehaviorRunning(baseInputProvider* provider) {
	inputProvider_ = provider;
}

//===================================================================================
//　初期化
//===================================================================================
void BehaviorRunning::Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) {
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

//===================================================================================
//　更新
//===================================================================================
void BehaviorRunning::Update(PlayableCharacterInfo& characterInfo) {

	//移動方向の正規化
	if (characterInfo.moveDirection.x != 0.0f || characterInfo.moveDirection.z != 0.0f) {
		characterInfo.moveDirection = Vector3Math::Normalize(characterInfo.moveDirection);
		//移動時の加速度の計算
		characterInfo.velocity.x += characterInfo.moveDirection.x * characterInfo.moveSpeed * deltaTime_;
		characterInfo.velocity.z += characterInfo.moveDirection.z * characterInfo.moveSpeed * deltaTime_;

		//パーティクルエミッターの更新
		//backEmitter_->Emit();

	} else {
		//速度の減速処理
		characterInfo.velocity.x /= characterInfo.deceleration;
		characterInfo.velocity.z /= characterInfo.deceleration;
	}

	//最大移動速度の制限
	float speed = sqrt(characterInfo.velocity.x * characterInfo.velocity.x + characterInfo.velocity.z * characterInfo.velocity.z);
	if (speed > characterInfo.kMaxMoveSpeed) {
		float scale = characterInfo.kMaxMoveSpeed / speed;
		characterInfo.velocity.x *= scale;
		characterInfo.velocity.z *= scale;
	}

	//移動処理
	// 位置の更新（deltaTimeをここで適用）
	characterInfo.transform.translate += characterInfo.velocity * deltaTime_;

}