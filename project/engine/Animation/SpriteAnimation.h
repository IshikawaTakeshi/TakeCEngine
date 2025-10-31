#pragma once
#include "engine/math/Easing.h"
#include "engine/math/Vector2.h"

//前方宣言
class Sprite;
namespace SpriteAnim {

	enum class PlayMode {
		LOOP,       //ループ再生
		ONCE,       //1回再生
		PINGPONG,   //往復再生
	};

	//拡大アニメーション
	void UpScale(Sprite& sprite, const Vector2& startSize, const Vector2& endSize, float duration,Easing::EasingType EaseType, PlayMode playMode);
	//縮小アニメーション
	void DownScale(Sprite& sprite, const Vector2& startSize, const Vector2& endSize, float duration, Easing::EasingType EaseType, PlayMode playMode);
	//移動アニメーション
	void MovePosition(Sprite& sprite, const Vector2& startPos, const Vector2& endPos, float duration, Easing::EasingType EaseType, PlayMode playMode);
	//フェードインアニメーション
	void FadeIn(Sprite& sprite, float duration, Easing::EasingType EaseType, PlayMode playMode);
}