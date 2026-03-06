#include "StateBreakStun.h"
#include "engine/math/Vector3Math.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　コンストラクタ
//===================================================================================
StateBreakStun::StateBreakStun(baseInputProvider* provider) {
	inputProvider_ = provider; //入力プロバイダーの設定
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); //デルタタイムの取得
	gravity_ = 9.8f; //重力の強さ
}

//===================================================================================
// 初期化
//===================================================================================

void StateBreakStun::Initialize([[maybe_unused]]PlayableCharacterInfo& characterInfo) {
	deltaTime_ = TakeCFrameWork::GetDeltaTime(); //デルタタイムの取得

	// ブレイクスタンのタイマーをリセットして開始
	breakStunTimer_.Initialize(characterInfo.breakGaugeInfo.stunDuration, 0.0f);
	// ブレイクスタン状態に設定
	characterInfo.breakGaugeInfo.isStunned = true; 

	if (characterInfo.characterName == "Player") {
		// ブレイクスタン開始のイベントを発行
		TakeCFrameWork::GetEventManager()->PostEvent("Initialize_BreakStunState_Player", &characterInfo.breakGaugeInfo);
	}
	else {
		// ブレイクスタン開始のイベントを発行
		TakeCFrameWork::GetEventManager()->PostEvent("Initialize_BreakStunState_Enemy", &characterInfo.breakGaugeInfo);
	}
}

//===================================================================================
// 更新
//===================================================================================

void StateBreakStun::Update(PlayableCharacterInfo& characterInfo) {

	// 速度を急激に落とす
	characterInfo.velocity.x *= 0.3f;
	characterInfo.velocity.z *= 0.3f;

	// ブレイクスタンのタイマーを更新
	breakStunTimer_.Update();

	if (breakStunTimer_.IsFinished()) {
		characterInfo.breakGaugeInfo.isStunned = false; // ブレイクスタン終了
		characterInfo.breakGaugeInfo.breakGauge = 0.0f; // ブレイクゲージをリセット

		//ゲージリセットのイベントを発行
		if (characterInfo.characterName == "Player") {
			TakeCFrameWork::GetEventManager()->PostEvent("BreakGaugeUpdate_Reset_Player", &characterInfo.breakGaugeInfo);
		}
		else {
			TakeCFrameWork::GetEventManager()->PostEvent("BreakGaugeUpdate_Reset_Enemy", &characterInfo.breakGaugeInfo);
		}
		// 地上なら RUNNING、空中なら FLOATING へ
		if (characterInfo.onGround) {
			nextState_ = GameCharacterState::RUNNING;
		}
		else {
			nextState_ = GameCharacterState::FLOATING;
		}
		isTransition_ = true;
	}

	// 移動速度
	Vector3& velocity_ = characterInfo.velocity; 
	// 空中での降下処理(fallSpeedを重力に加算)
	if (!characterInfo.onGround) {
		velocity_.y -= (gravity_ + characterInfo.fallSpeed) * deltaTime_;
		characterInfo.transform.translate += velocity_ * deltaTime_;
	}
}