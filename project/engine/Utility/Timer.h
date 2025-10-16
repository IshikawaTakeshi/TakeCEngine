#pragma once
#include "engine/math/Easing.h"

class Timer {
public:

	Timer() = default;
    Timer(float _duration, float current);
	~Timer() = default;

    void Initialize(float _duration,float current);
    float GetProgress() const;
    float GetDuration() const;
    float GetEase(Easing::EasingType easeType);
    float GetPrevProgress() const;
    bool IsFinished() const;
    bool IsFinishedNow() const;
    void Reset();
    void ToEnd();
    void Stop();
    void Restart();
    void Update(float timeScale = 1.0f);

private:

	bool isStop_ = false;
	float currentTime_ = 0.0f;
	float prevTime_ = 0.0f;
	float duration_ = 0.0f;

};