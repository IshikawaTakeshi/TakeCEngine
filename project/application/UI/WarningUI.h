#pragma once
#include "engine/2d/BaseUI.h"
#include "engine/Utility/Timer.h"

//危険表示の方向
enum class WarningDirection {
	FRONT,
	RIGHT,
	BACK,
	LEFT,
	COUNT
};

class Camera;

//============================================================================
// WarningUI class
//============================================================================
class WarningUI : public BaseUI {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	WarningUI() = default;
	~WarningUI() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::SpriteManager* spriteManager,const std::string& configName);

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	void UpdateImGui(const std::string& name) override;

private:
	// 方向計算
	WarningDirection CalculateDirection(const Vector3& targetPos);

	// 指定方向の警告を開始
	void TriggerWarning(WarningDirection dir);


private:

	Camera* targetCamera_ = nullptr;

	// 各方向ごとの状態管理用構造体
	struct DirectionState {
		Sprite* spritePtr = nullptr; 

		Timer timer;        // 点滅タイマー
		float currentAlpha = 0.0f; // 現在のアルファ値
		bool isFlashing = false;   // 点滅中かどうか
	};

	// 6方向分の状態
	std::array<DirectionState, static_cast<int>(WarningDirection::COUNT)> directionStates_;

	// 設定定数
	const float kFlashDuration_ = 2.0f; // 点滅継続時間（秒）
	const float kFlashSpeed_ = 60.0f;   // 点滅の速さ
};

