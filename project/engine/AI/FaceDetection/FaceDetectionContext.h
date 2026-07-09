#pragma once
#include "engine/Math/Vector2.h"


/// <summary>
/// 顔のランドマーク5点
/// </summary>
struct FaceLandmark5 {
    Vector2 leftEye;
    Vector2 rightEye;
    Vector2 nose;
    Vector2 leftMouth;
    Vector2 rightMouth;
};

/// <summary>
/// 顔検出結果
/// </summary>
struct FaceDetectionResult {
    float score = 0.0f;
    Vector2 bboxMin;
    Vector2 bboxMax;
    FaceLandmark5 landmark;
};