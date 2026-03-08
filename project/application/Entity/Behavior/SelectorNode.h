#pragma once
#include "application/Entity/Behavior/CompositeNode.h"

//==================================================================================
// SelectorNode class
// 子ノードを順番に実行し、最初の成功したノードで成功を返す
//==================================================================================
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
	BehaviorStatus Execute(PlayableCharacterInfo& characterInfo) override;

};