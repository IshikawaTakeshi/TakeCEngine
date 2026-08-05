#pragma once
#include "BehaviorStatusEnum.h"
#include "Blackboard.h"
#include <string>
#include <string_view>
#include <utility>

//==================================================================================
// BehaviorNode class
// 全ノード共通のインターフェース
//==================================================================================

/// <summary>
/// ビヘイビアツリーを構成する全ノード共通の実行インターフェースを提供する基底クラスです。
/// </summary>
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
	/// 現在の状態を取得
	/// </summary>
	BehaviorStatus GetCurrentStatus() const { return currentStatus_; }

	/// <summary>
	/// UIDの設定
	/// </summary>
	void SetUID(int uid) { uid_ = uid; }

	/// <summary>
	/// UIDの取得
	/// </summary>
	int GetUID() const { return uid_; }

	/// <summary>
	/// レジストリに登録されたノード型IDを設定します。
	/// </summary>
	void SetTypeId(std::string typeId) { typeId_ = std::move(typeId); }

	/// <summary>
	/// レジストリに登録されたノード型IDを取得します。
	/// </summary>
	std::string_view GetTypeId() const noexcept { return typeId_; }

protected:
	
	// デバッグ用の名前
	std::string name_ = "UnnamedNode";

	// 保存データとの紐付け用の一意識別子
	int uid_ = -1;

	// レジストリで使用する永続的なノード型ID
	std::string typeId_;

	// 実行状態を保持するメンバ
	BehaviorStatus currentStatus_ = BehaviorStatus::Invalid;
};
