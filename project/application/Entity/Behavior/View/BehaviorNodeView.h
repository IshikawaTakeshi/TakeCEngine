#pragma once
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include "application/Entity/Behavior/BehaviorStatusEnum.h"
#include <string>

//=============================================================
//BehaviorNodeView class
//=============================================================
class BehaviorNodeView :public ImFlow::BaseNode {
public:
	
	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BehaviorNodeView() = default;
	~BehaviorNodeView() override = default;

	//=========================================================
	// functions
	//=========================================================

	void draw() override;


protected:

	/// ノードの状態
	BehaviorStatus lastStatus_ = BehaviorStatus::Invalid;

	/// <summary>
	/// ノードの状態に応じた色を取得
	/// </summary>
	static ImVec4 statusToColor(BehaviorStatus status);
};

