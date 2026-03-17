#pragma once
#include "BehaviorNodeView.h"
#include "application/Entity/State/GameCharacterState.h"
#include <string>

//=============================================================
//BehaviorNodeView class
//=============================================================
class ActionNodeView : public BehaviorNodeView {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	/// <param name="stateName"></param>
	ActionNodeView(const std::string& stateName = "NONE");
	~ActionNodeView() override = default;

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

	const std::string& GetTargetStateName() const { return targetStateName_; }
	void SetTargetStateName(const std::string& stateName) { targetStateName_ = stateName; }

private:
	std::string targetStateName_;  // "ATTACK", "RUNNING" 等
};