#include "SpriteAnimation.h"
#include "engine/Utility/Timer.h"
#include "engine/2d/Sprite.h"
#include <utility>

//=============================================================================
// 拡大アニメーション
//=============================================================================

void SpriteAnim::UpScale(Sprite& sprite, const Vector2& startSize, const Vector2& endSize, float duration, Easing::EasingType EaseType, PlayMode playMode) {

	//タイマーの更新
	static Timer timer(duration, 0.0f);
	timer.Update();
	//進捗の計算
	float t = timer.GetEase(EaseType);
	//スケールの計算
	Vector2 newScale = Easing::Lerp(startSize, endSize, t);
	sprite.SetSize(newScale);
	//再生モードによる処理
	if (playMode == PlayMode::LOOP) {
		if (timer.IsFinished()) {
			timer.Reset();
		}
	}
	else if (playMode == PlayMode::ONCE) {
		if (timer.IsFinished()) {
			sprite.SetSize(endSize);
		}
	} else if (playMode == PlayMode::PINGPONG) {
		if (timer.IsFinished()) {
			DownScale(sprite, endSize, startSize, duration, EaseType, PlayMode::ONCE);
		}
	}
}

void SpriteAnim::DownScale(Sprite& sprite, const Vector2& startSize, const Vector2& endSize, float duration, Easing::EasingType EaseType, PlayMode playMode) {
	//タイマーの更新
	static Timer timer(duration, 0.0f);
	timer.Update();
	//進捗の計算
	float t = timer.GetEase(EaseType);
	//スケールの計算
	Vector2 newScale = Easing::Lerp(startSize, endSize, t);
	sprite.SetSize(newScale);
	//再生モードによる処理
	if (playMode == PlayMode::LOOP) {
		if (timer.IsFinished()) {
			timer.Reset();
		}
	}
	else if (playMode == PlayMode::ONCE) {
		if (timer.IsFinished()) {
			sprite.SetSize(endSize);
		}
	}
	else if (playMode == PlayMode::PINGPONG) {
		if (timer.IsFinished()) {

			timer.Reset();
		}
	}
}
