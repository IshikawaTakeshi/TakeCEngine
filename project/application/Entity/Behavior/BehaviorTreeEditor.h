#pragma once
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include <map>
#include "application/Entity/Behavior/BehaviorNode.h"
#include "application/Entity/Behavior/CompositeNode.h"
#include "application/Entity/Behavior/SelectorNode.h"
#include "application/Entity/Behavior/SequenceNode.h"
#include "application/Entity/Behavior/ConditionNode.h"
#include "application/Entity/Behavior/ScoreConditionNode.h"
#include "application/Entity/Behavior/PlannerSelectorNode.h"
#include "application/Entity/Behavior/ActionNode.h"
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
	void UpdateImGui();

	/// <summary>
	/// Enemyからビヘイビアツリーを読み込む
	/// </summary>
	/// <param name="rootNode">Enemyが保持しているルートノード</param>
	void LoadTreeFromEnemy(BehaviorNode* rootNode);

private:

	/// <summary>
	/// 再帰的にノードを構築する
	/// </summary>
	ImFlow::BaseNode* BuildNodeView(BehaviorNode* node, ImVec2& minPos);

private:

	std::unique_ptr<ImFlow::ImNodeFlow> flowEditor_; // ImNodeFlowのエディタ
	std::map<BehaviorNode*, ImFlow::BaseNode*> nodeViewMap_; // 実際のノードとViewの紐づけ
};