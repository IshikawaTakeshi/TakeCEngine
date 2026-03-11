#pragma once
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include "application/Entity/Behavior/View/ActionNodeView.h"
#include "application/Entity/Behavior/View/SelectorNodeView.h"
#include "application/Entity/Behavior/View/ConditionNodeView.h"
#include "application/Entity/Behavior/View/SequenceNodeView.h"
#include "application/Entity/Behavior/View/ScoreConditionNodeView.h"
#include "application/Entity/Behavior/View/PlannerSelectorNodeView.h"
#include "application/Entity/Behavior/View/WeightSelectorNodeView.h"

//==================================================================================
// BehaviorTreeEditor class
//==================================================================================
class BehaviorTreeEditor {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BehaviorTreeEditor() = default;
	~BehaviorTreeEditor() = default;
	//====================================================================================
	// functions
	//====================================================================================

	void Initialize();

	/// <summary>
	/// ビヘイビアツリーエディタの描画
	/// </summary>
	void Draw();

private:

	std::unique_ptr<ImFlow::ImNodeFlow> flowEditor_; // ImNodeFlowのエディタ
};