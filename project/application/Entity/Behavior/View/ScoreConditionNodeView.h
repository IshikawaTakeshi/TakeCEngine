#pragma once
#include "BehaviorNodeView.h"
#include <string>

//=============================================================
// ScoreConditionNodeView class
//=============================================================
class ScoreConditionNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	/// <param name="threshold"></param>
	ScoreConditionNodeView(float threshold = 0.0f);
	~ScoreConditionNodeView() override = default;

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// ノードの内容描画
	/// </summary>
	void draw() override;

	//=========================================================
	// accsesser
	//=========================================================

	float GetThreshold() const { return threshold_; }
	void SetThreshold(float threshold) { threshold_ = threshold; }

private:
	float threshold_;  // スコア閾値
};
