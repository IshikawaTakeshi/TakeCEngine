#pragma once
#include "engine/2d/Sprite.h"
#include "engine/math/Vector2.h"
#include "engine/math/Vector4.h"
#include <memory>
#include <array>
#include <string>

/// <summary>
/// 4分割されたリングを表すUI要素。
/// 4つの回転したスプライトを使用してリングを形成します。
/// </summary>
class RingUI {
public:
    /// <summary>
    /// Ring UIの初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// Ring UIの更新
    /// </summary>
    void Update();

    /// <summary>
    /// Ring UIの描画
    /// </summary>
    void Draw();

    // 特定のパーツ(0-3)の色を設定
    void SetColor(int index, const Vector4& color);
    
    // 全てのパーツの色を設定
    void SetAllColor(const Vector4& color);

    // 特定のパーツのスケールを設定
    void SetScale(int index, const Vector2& scale);

    // リングの中心位置を設定
    void SetCenterPosition(const Vector2& position);

    // 表示/非表示の設定
    void SetActive(bool isActive);

private:
    static const int kPartCount = 4;
    std::array<std::unique_ptr<Sprite>, kPartCount> parts_;
    Vector2 centerPos_ = { 640.0f, 360.0f }; // デフォルトの中心位置
    bool isActive_ = true;
};
