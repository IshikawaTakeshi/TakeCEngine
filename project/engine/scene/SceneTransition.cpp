#include "SceneTransition.h"
#include "2d/SpriteCommon.h"
#include "base/WinApp.h"
#include "base/TakeCFrameWork.h"
#include <algorithm>

SceneTransition* SceneTransition::instance_ = nullptr;

//=============================================================================
//初期化
//=============================================================================

SceneTransition* SceneTransition::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new SceneTransition();
	}
	return instance_;
}

void SceneTransition::Initialize() {

	state_ = TransitionState::NONE;

	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(SpriteCommon::GetInstance(), "black.png");
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

	//state_ = TransitionState::NONE;
	fadeSprite_.reset();

	if(instance_ != nullptr) {
		instance_ = nullptr;
	}
}

//=============================================================================
//更新処理
//=============================================================================

void SceneTransition::Update() {

	switch (state_) {
	case TransitionState::NONE:
		break;

	case TransitionState::FADE_IN:

		//
		transitionTime_ -= transitionSpeed_;

		//
		if (transitionTime_ <= 0.0f) {
			transitionTime_ = 0.0f;
			state_ = TransitionState::NONE;
		}

		alpha_ = std::clamp(transitionTime_ / duration_, 0.0f, 1.0f);
		break;

	case TransitionState::FADE_OUT:

		//
		transitionTime_ += transitionSpeed_;

		//
		if (transitionTime_ >= duration_) {
			transitionTime_ = duration_;
		}

		alpha_ = std::clamp(transitionTime_ / duration_, 0.0f, 1.0f);
		break;
	}

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

	if (state_ == TransitionState::NONE) {
		return;
	}

	SpriteCommon::GetInstance()->PreDraw();
	fadeSprite_->Draw();
}

//=============================================================================
//シーン遷移開始
//=============================================================================

void SceneTransition::Start(TransitionState state, float duration) {

	state_ = state;
	duration_ = duration;

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

bool SceneTransition::IsFinished() {
	switch (state_) {
	case TransitionState::FADE_IN:
	case TransitionState::FADE_OUT:
		if (transitionTime_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}