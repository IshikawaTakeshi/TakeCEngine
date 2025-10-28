#pragma once
#include "engine/math/Easing.h"

//============================================================================
// Timer class
//============================================================================
class Timer {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Timer() = default;

    /// <summary>
	/// コンストラクタ(期間、開始時間指定)
    /// </summary>
    /// <param name="_duration"></param>
    /// <param name="current"></param>
    Timer(float _duration, float current);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Timer() = default;
    
    /// <summary>
	/// 初期化
    /// </summary>
    /// <param name="_duration"></param>
    /// <param name="current"></param>
    void Initialize(float _duration,float current);

    /// <summary>
	/// 進捗の取得(0.0~1.0)
    /// </summary>
    /// <returns></returns>
    float GetProgress() const;

    /// <summary>
	/// 期間の取得
    /// </summary>
    /// <returns></returns>
    float GetDuration() const;

    /// <summary>
	/// イージング適用後の進捗の取得
    /// </summary>
    /// <param name="easeType"></param>
    /// <returns></returns>
    float GetEase(Easing::EasingType easeType);

    /// <summary>
	/// 前回の進捗の取得(0.0~1.0)
    /// </summary>
    /// <returns></returns>
    float GetPrevProgress() const;

    /// <summary>
	/// 終了判定
    /// </summary>
    /// <returns></returns>
    bool IsFinished() const;

    /// <summary>
	/// 即時終了判定
    /// </summary>
    /// <returns></returns>
    bool IsFinishedNow() const;

    /// <summary>
	/// リセット
    /// </summary>
    void Reset();

    /// <summary>
	/// 最後まで進める
    /// </summary>
    void ToEnd();

    /// <summary>
	/// 停止処理
    /// </summary>
    void Stop();
    
    /// <summary>
	/// 再開処理
    /// </summary>
    void Restart();

    /// <summary>
	/// 更新処理
    /// </summary>
    /// <param name="timeScale"></param>
    void Update(float timeScale = 1.0f);

private:

	//停止フラグ
	bool isStop_ = false;
	//現在時間
	float currentTime_ = 0.0f;
	//前回時間
	float prevTime_ = 0.0f;
	//期間
	float duration_ = 0.0f;

};