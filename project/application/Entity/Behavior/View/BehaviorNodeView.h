#pragma once
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include "application/Entity/Behavior/BehaviorStatusEnum.h"
#include "application/Entity/Behavior/BehaviorTreeUtil.h"

#include "engine/Math/Vector4.h"
#include <string>

class BehaviorNode;

//=============================================================
//BehaviorNodeView class
//=============================================================
class BehaviorNodeView : public ImFlow::BaseNode {
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
	
	/// <summary>
	/// 入力ピンを取得
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	ImFlow::Pin* GetInputPin(size_t index) const;

	/// <summary>
	/// 入力ピンを追加
	/// </summary>
	/// <param name="pin"></param>
	void AddInputPin(ImFlow::Pin* pin);

	/// <summary>
	/// 入力ピンの数を取得
	/// </summary>
	/// <returns></returns>
	size_t GetInputPinCount() const { return inPins_.size(); }

	/// <summary>
	/// 出力ピンを取得
	/// </summary>
	ImFlow::Pin* GetOutputPin(size_t index) const;

	/// <summary>
	/// 出力ピンを追加
	/// </summary>
	/// <param name="pin"></param>
	void AddOutputPin(ImFlow::Pin* pin);

	/// <summary>
	/// 出力ピンの数を取得
	/// </summary>
	/// <returns></returns>
	size_t GetOutputPinCount() const { return outPins_.size(); }

	void SetCurrentStatus(BehaviorStatus status) { currentStatus_ = status; }

	//=========================================================================
	// serialization [EXT]
	//=========================================================

	/// <summary>
	/// ノードの一意識別子を設定
	/// </summary>
	void SetNodeUID(int uid) { nodeUID_ = uid; }

	/// <summary>
	/// ノードの一意識別子を取得
	/// </summary>
	int GetNodeUID() const { return nodeUID_; }

	/// <summary>
	/// ノードタイプ文字列を取得
	/// </summary>
	virtual std::string GetNodeType() const = 0;

	/// <summary>
	/// パラメータをデータ構造に移す
	/// </summary>
	virtual void SaveParameters(BehaviorNodeData& data) const { data.nodeType = GetNodeType(); }

	/// <summary>
	/// データ構造からパラメータを読み込む
	/// </summary>
	virtual void LoadParameters(const BehaviorNodeData& data) { (void)data; }

	//=========================================================================
	// logic link
	//=========================================================================

	/// <summary>
	/// ロジックノードを紐づける
	/// </summary>
	/// <param name="node"></param>
	void SetLogicNode(BehaviorNode* node) { logicNode_ = node; }


protected:

	// 紐づくロジックノード
	BehaviorNode* logicNode_ = nullptr;

	// データ紐付け用のUID [NEW]
	int nodeUID_ = -1;

	/// ノードの状態
	BehaviorStatus currentStatus_ = BehaviorStatus::Invalid;

	/// <summary>
	/// ノードの状態に応じた色を取得
	/// </summary>
	/// ノードの状態に応じた色を取得
	static ImVec4 statusToColor(BehaviorStatus status);

	/// 入力ピン
	std::vector<ImFlow::Pin*> inPins_;

	/// 出力ピン
	std::vector<ImFlow::Pin*> outPins_;
};


