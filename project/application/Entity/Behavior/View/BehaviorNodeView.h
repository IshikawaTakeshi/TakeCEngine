#pragma once
#include <ImNodeFlow-1.2.2/include/ImNodeFlow.h>
#include "application/Entity/Behavior/BehaviorStatusEnum.h"
#include "engine/Math/Vector4.h"
#include <string>

//=============================================================
//BehaviorNodeView class
//=============================================================
class BehaviorNodeView :public ImFlow::BaseNode {
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


protected:

	/// ノードの状態
	BehaviorStatus lastStatus_ = BehaviorStatus::Invalid;

	/// <summary>
	/// ノードの状態に応じた色を取得
	/// </summary>
	/// ノードの状態に応じた色を取得
	static ImVec4 statusToColor(BehaviorStatus status);

	/// 入力ピンを順序を維持して保持する
	std::vector<ImFlow::Pin*> inPins_;

	/// 出力ピンを順序を維持して保持する
	std::vector<ImFlow::Pin*> outPins_;
};

