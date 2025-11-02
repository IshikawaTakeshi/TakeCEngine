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

	if (target_) target_->SetSize(startSize_);

	// 🔹 アニメーション関数をラムダとして登録
	currentAnimFunc_ = [this](float deltaTime) {
		if (!target_) return;
		timer_ += deltaTime;

		switch (state_) {
		case State::Up:
		{
			float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);
			float eased = Easing::Ease[easeType_](t);
			Vector2 size = Easing::Lerp(startSize_, endSize_, eased);
			target_->SetSize(size);

			if (t >= 1.0f) {
				timer_ = 0.0f;
				if (playMode_ == PlayMode::PINGPONG) {
					state_ = State::Delay;
				} else if (playMode_ == PlayMode::LOOP) {
					state_ = State::Up;
					target_->SetSize(startSize_);
				} else if (playMode_ == PlayMode::ONCE) {
					state_ = State::None;
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
					state_ = State::Up;
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