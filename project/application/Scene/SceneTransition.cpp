#include "SceneTransition.h"
#include "2d/SpriteCommon.h"
#include "base/WinApp.h"
#include "base/TakeCFrameWork.h"
#include <algorithm>

//シングルトンインスタンスの初期化
SceneTransition* SceneTransition::instance_ = nullptr;

//=============================================================================
// シングルトンインスタンス取得
//=============================================================================

SceneTransition* SceneTransition::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new SceneTransition();
	}
	return instance_;
}

//=============================================================================
//初期化
//=============================================================================
void SceneTransition::Initialize() {

	state_ = TransitionState::NONE;

	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(&SpriteCommon::GetInstance(), "black.png");
	fadeSprite_->SetTextureSize({ WinApp::kScreenWidth,WinApp::kScreenHeight });
	fadeSprite_->SetSize({ WinApp::kScreenWidth,WinApp::kScreenHeight });
	duration_ = 0.0f;

	transitionTime_ = 0.0f;

	transitionSpeed_ = TakeCFrameWork::GetDeltaTime();

	alpha_ = 0.0f;
}

//=============================================================================
//終了処理
//=============================================================================

void SceneTransition::Finalize() {

	fadeSprite_.reset();

	if(instance_ != nullptr) {
		instance_ = nullptr;
	}
}

//=============================================================================
//更新処理
//=============================================================================

void SceneTransition::Update() {

	//状態に応じて処理
	switch (state_) {
	case TransitionState::NONE:
		break;

	case TransitionState::FADE_IN:

		//遷移時間を減少
		transitionTime_ -= transitionSpeed_;

		//時間が0以下になったらstateをNONEに
		if (transitionTime_ <= 0.0f) {
			transitionTime_ = 0.0f;
			state_ = TransitionState::NONE;
		}

		//アルファ値計算
		alpha_ = std::clamp(transitionTime_ / duration_, 0.0f, 1.0f);
		break;

	case TransitionState::FADE_OUT:

		//遷移時間を加算
		transitionTime_ += transitionSpeed_;

		//時間がduration以上になったらdurationに固定
		if (transitionTime_ >= duration_) {
			transitionTime_ = duration_;
		}

		//アルファ値計算
		alpha_ = std::clamp(transitionTime_ / duration_, 0.0f, 1.0f);
		break;
	}

	//スプライトの更新
	fadeSprite_->GetMesh()->GetMaterial()->SetMaterialColor({0.0f, 0.0f, 0.0f, alpha_});
	fadeSprite_->Update();
#ifdef _DEBUG

	fadeSprite_->UpdateImGui("fade");
#endif // _DEBUG

}

//=============================================================================
//描画処理
//=============================================================================

void SceneTransition::Draw() {

	//遷移中でなければ描画しない
	if (state_ == TransitionState::NONE) {
		return;
	}

	//描画前処理
	SpriteCommon::GetInstance().PreDraw();
	//スプライト描画
	fadeSprite_->Draw();
}

//=============================================================================
//シーン遷移開始
//=============================================================================

void SceneTransition::Start(TransitionState state, float duration) {


	state_ = state;
	duration_ = duration;

	//状態に応じて初期値設定
	if (state_ == TransitionState::FADE_IN) {

		alpha_ = 1.0f;
		transitionTime_ = duration_;
	} else if (state_ == TransitionState::FADE_OUT) {

		alpha_ = 0.0f;
		transitionTime_ = 0.0f;
	}
}

//=============================================================================
//シーン遷移を中止させる処理
//=============================================================================

void SceneTransition::Stop() {

	state_ = TransitionState::NONE;
}

//=============================================================================
//シーン遷移が終了したかどうか
//=============================================================================
bool SceneTransition::IsFinished() {
	switch (state_) {
	case TransitionState::FADE_IN:
	case TransitionState::FADE_OUT:

		//遷移時間がduration以上になったら終了
		if (transitionTime_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}