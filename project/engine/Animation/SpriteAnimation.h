#pragma once
#include <functional>
#include <vector>
#include "engine/math/Easing.h"
#include "engine/math/Vector2.h"
#include "engine/Utility/Timer.h"



//前方宣言
class Sprite;

//============================================================================
// SpriteAnimator class
//============================================================================
class SpriteAnimator {
public:

	///状態列挙型
	enum class AnimationPhase {
		None,
		Up,
		Delay,
		Down
	};

	///再生モード列挙型
	enum class PlayMode {
		LOOP,
		ONCE,
		PINGPONG_LOOP,
		PINGPONG_ONCE
	};

public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SpriteAnimator() = default;
	~SpriteAnimator() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="target"></param>
	void Initialize(Sprite* target);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 拡大アニメーションの再生
	/// </summary>
	/// <param name="target"></param>
	/// <param name="startSize"></param>
	/// <param name="endSize"></param>
	/// <param name="duration"></param>
	/// <param name="delay"></param>
	/// <param name="easeType"></param>
	/// <param name="playMode"></param>
	void PlayUpScale(const Vector2& startSize, const Vector2& endSize, float duration, float delay, Easing::EasingType easeType, PlayMode playMode);

	/// <summary>
	/// 移動アニメーションの再生
	/// </summary>
	/// <param name="startPos"></param>
	/// <param name="endPos"></param>
	/// <param name="duration"></param>
	/// <param name="delay"></param>
	/// <param name="easeType"></param>
	/// <param name="playMode"></param>
	void PlayTranslate(const Vector2& startPos, const Vector2& endPos, float duration, float delay, Easing::EasingType easeType, PlayMode playMode);

	void PlayFade(const float startAlpha, const float endAlpha, float duration, float delay, Easing::EasingType easeType, PlayMode playMode);

	/// <summary>
	/// アニメーション再生中かどうか
	/// </summary>
	/// <returns></returns>
	bool IsPlaying() const;

	/// <summary>
	/// アニメーションが終了したかどうか
	/// </summary>
	/// <returns></returns>
	bool IsFinished() const;

private:

	//========================================================================
	// variables
	//========================================================================

	///アニメーションジョブ構造体
	struct AnimationJob {
		int id = 0;

		//アニメーション用タイマー
		Timer timer;
		float delay = 0.0f;

		//アニメーション状態
		AnimationPhase state = AnimationPhase::None;
		//イージングタイプ
		Easing::EasingType easeType = Easing::EasingType::LINEAR;
		//再生モード
		PlayMode playMode = PlayMode::ONCE;

		//Vec2用変数
		Vector2 startValueVec2{};
		Vector2 endValueVec2{};
		//float用変数
		float startValueFloat = 0.0f;
		float endValueFloat = 0.0f;

		//更新関数(ラムダで格納)(Vec2用・float用)
		std::function<void(AnimationJob&)> updateFunc;

		//終了フラグ
		bool finished = false;
	};

	//アニメーションさせるスプライト
	Sprite* target_ = nullptr;
	
	// 現在実行するアニメーション関数（ラムダで格納）
	std::vector<AnimationJob> animationJobs_;
	int nextJobId_ = 1;
};