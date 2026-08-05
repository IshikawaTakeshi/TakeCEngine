#pragma once
#include "CompositeNode.h"

//==================================================================================
// SelectorNode class
// 子ノードを順番に実行し、最初の成功したノードで成功を返す
//==================================================================================
/// <summary>
/// ビヘイビアツリーのSelectorNodeとして判定または処理を実行するクラスです。
/// </summary>
class SelectorNode : public CompositeNode {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	SelectorNode() { name_ = "SelectorNode"; }
	~SelectorNode() override = default;

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
