#include "SpriteAnimation.h"
#include "engine/Utility/Timer.h"
#include "engine/2d/Sprite.h"
#include "engine/base/TakeCFrameWork.h"
#include <utility>

void SpriteAnimator::Initialize(Sprite* target) {
	target_ = target;
}

//=============================================================================
// 拡大アニメーション
//=============================================================================

void SpriteAnimator::PlayUpScale(
	const Vector2& startSize, const Vector2& endSize,
	float duration, float delay,
	Easing::EasingType easeType, PlayMode playMode) {

	startSize_ = startSize;
	endSize_ = endSize;
	duration_ = duration;
	delay_ = delay;
	easeType_ = easeType;
	playMode_ = playMode;
	timer_ = 0.0f;
	state_ = State::Up;

	//アニメーション開始時にサイズを初期化
	if (target_) target_->SetSize(startSize_);

	//アニメーション関数をラムダとして登録
	currentAnimFunc_ = [this](float deltaTime) {
		if (!target_) return;
		timer_ += deltaTime;

		//状態別処理
		switch (state_) {
		case State::Up:
		{

			
			float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);
			float eased = Easing::Ease[easeType_](t);
			Vector2 size = Easing::Lerp(startSize_, endSize_, eased);
			target_->SetSize(size);

			// 完了判定
			if (t >= 1.0f) {
				timer_ = 0.0f;
				if (playMode_ == PlayMode::PINGPONG) {
					state_ = State::Delay; // PINGPONGの場合はDelayへ
				} else if (playMode_ == PlayMode::LOOP) {
					state_ = State::Up;    // LOOPの場合は再度Upへ
					target_->SetSize(startSize_);
				} else if (playMode_ == PlayMode::ONCE) {
					state_ = State::None;  // ONCEの場合は終了
					target_->SetSize(endSize_);
				}
			}
			break;
		}

		case State::Delay:
		{
			if (timer_ >= delay_) {
				timer_ = 0.0f;
				state_ = State::Down;
			}
			break;
		}

		case State::Down:
		{
			float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);
			float eased = Easing::Ease[easeType_](t);
			Vector2 size = Easing::Lerp(endSize_, startSize_, eased);
			target_->SetSize(size);

			if (t >= 1.0f) {
				timer_ = 0.0f;
				if (playMode_ == PlayMode::PINGPONG) {
					state_ = State::None;
					target_->SetSize(startSize_);
				} else if (playMode_ == PlayMode::LOOP) {
					state_ = State::Up;
					target_->SetSize(startSize_);
				} else if (playMode_ == PlayMode::ONCE) {
					state_ = State::None;
					target_->SetSize(startSize_);
				}
			}
			break;
		}

		default:
			break;
		}
		};
}

void SpriteAnimator::Update(float deltaTime) {
	if (state_ != State::None && currentAnimFunc_) {
		currentAnimFunc_(deltaTime);
	}
}