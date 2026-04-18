#pragma once
#include "application/Entity/Behavior/CompositeNode.h"

//==================================================================================
// SequenceNode class
// 子ノードを順番に実行し、全て成功したら成功
// どれか失敗したら失敗、実行中のノードがあれば実行中を返す
//==================================================================================
class SequenceNode : public CompositeNode {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SequenceNode() { name_ = "SequenceNode"; }
	~SequenceNode() override = default;

	//==============================================================================
	// functions
	//==============================================================================

	/// <summary>
	/// ノードの実行
	/// </summary>
	/// <param name="characterInfo"></param>
	/// <returns></returns>
	BehaviorStatus Execute(Blackboard& blackboard) override;
};

