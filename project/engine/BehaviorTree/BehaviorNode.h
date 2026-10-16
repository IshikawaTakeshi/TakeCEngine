#pragma once
#include "BehaviorStatusEnum.h"
#include "Blackboard.h"
#include "BehaviorTreeContext.h"
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
	// 旧Blackboard専用の実行入口。新しいノードはTick(AIContext&)だけを実装できます。
	// 未実装の入口を誤って呼んだ場合はFailureを返します。
	/// </summary>
	virtual BehaviorStatus Execute([[maybe_unused]] Blackboard& blackboard) {
		currentStatus_ = BehaviorStatus::Failure;
		return currentStatus_;
	}

	/// <summary>AIContext対応ノードの実行入口。旧const Tickにも委譲できます。</summary>
	virtual BehaviorStatus Tick(TakeC::AIContext& context) {
		return Tick(static_cast<const TakeC::AIContext&>(context));
	}

	/// <summary>旧Contextノードとの互換入口。Blackboard専用ノードはExecuteを使用します。</summary>
	virtual BehaviorStatus Tick(const TakeC::BehaviorTreeContext& context) {
		return Execute(context.blackboard);
	}

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
