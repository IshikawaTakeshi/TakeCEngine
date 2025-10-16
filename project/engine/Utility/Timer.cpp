#include "Timer.h"
#include "engine/base/TakeCFrameWork.h"
#include <algorithm>

Timer::Timer(float _duration, float current) {
	Initialize(_duration, current);
}

void Timer::Initialize(float _duration, float current) {
	duration_ = _duration;
	currentTime_ = current;
	prevTime_ = currentTime_;
	isStop_ = false;
}

float Timer::GetProgress() const {
	return currentTime_ / duration_;
}

float Timer::GetDuration() const {
	return duration_;
}

float Timer::GetEase(Easing::EasingType easeType) {
	return Easing::Ease[easeType](GetProgress());
}

float Timer::GetPrevProgress() const {
	return prevTime_ / duration_;
}

bool Timer::IsFinished() const {
	return currentTime_ >= duration_;
}

bool Timer::IsFinishedNow() const {
	return currentTime_ >= duration_;
}

void Timer::Reset() {
	currentTime_ = 0.0f;
}

void Timer::ToEnd() {
	currentTime_ = duration_;
}

void Timer::Restart() {
	currentTime_ = 0.0f;
	isStop_ = false;
}

void Timer::Update(float timeScale) {
	prevTime_ = currentTime_;
	currentTime_ = std::clamp(currentTime_ + TakeCFrameWork::GetDeltaTime() * timeScale * !isStop_, 0.0f, duration_);
}
