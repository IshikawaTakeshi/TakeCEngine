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
	virtual void Reset() {
		currentStatus_ = BehaviorStatus::Invalid;
	}

	/// <summary>
	/// ImGuiを用いたデバッグ用情報の描画
	/// </summary>
	virtual void DrawInspector() {}

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

	/// <summary>
	/// 現在の状態を取得 [EXT]
	/// </summary>
	BehaviorStatus GetCurrentStatus() const { return currentStatus_; }

	/// <summary>
	/// UIDの設定 [NEW]
	/// </summary>
	void SetUID(int uid) { uid_ = uid; }

	/// <summary>
	/// UIDの取得 [NEW]
	/// </summary>
	int GetUID() const { return uid_; }

protected:

	std::string name_ = "UnnamedNode";

	// 保存データとの紐付け用の一意識別子
	int uid_ = -1;

	// 実行状態を保持するメンバ
	BehaviorStatus currentStatus_ = BehaviorStatus::Invalid;
};