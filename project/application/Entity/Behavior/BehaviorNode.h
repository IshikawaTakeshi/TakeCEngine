#pragma once
#include "application/Entity/Behavior/BehaviorStatusEnum.h"
#include "application/Entity/Behavior/Blackboard.h"
#include <vector>
#include <memory>
#include <string>
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>

//==================================================================================
// BehaviorNode class
// 全ノード共通のインターフェース
//==================================================================================

class BehaviorNode {
public:

	/// <summary>
	/// コンストラクタとデストラクタ
	/// </summary>
	BehaviorNode() = default;
	virtual ~BehaviorNode() = default;

	//====================================================================================
	// functions
	//====================================================================================

	/// <summary>
	// ノードの実行
	/// </summary>
	virtual BehaviorStatus Execute(Blackboard& blackboard) = 0;

	/// <summary>
	// ノードのリセット（再実行時に状態をクリア）
	/// </summary>
	virtual void Reset() {}

	//====================================================================================
	// accessors
	//====================================================================================

	/// <summary>
	// デバッグ用の名前
	/// </summary>
	void SetName(const std::string& name) { name_ = name; }

	/// <summary>
	/// デバッグ用の名前を取得
	/// </summary>
	/// <returns></returns>
	const std::string& GetName() const { return name_; }

protected:

	std::string name_ = "UnnamedNode";
};