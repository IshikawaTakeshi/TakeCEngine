#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"
#include <vector>
#include <memory>

//============================================================================
// CompositeNode class
// 子を持つノードの共通基底クラス
//============================================================================
class CompositeNode : public BehaviorNode {
public:

	/// <summary>
	/// コンストラクタとデストラクタ
	/// </summary>
	CompositeNode() = default;
	~CompositeNode() override = default;

	//=============================================================================
	// functions
	//=============================================================================

	/// <summary>
	/// 子ノードの追加
	/// </summary>
	/// <param name="child"></param>
	void AddChild(std::unique_ptr<BehaviorNode> child) {
		children_.push_back(std::move(child));
	}

	/// <summary>
	/// 子ノードのリストを取得する
	/// </summary>
	const std::vector<std::unique_ptr<BehaviorNode>>& GetChildren() const { return children_; }

	/// <summary>
	/// ノードのリセット
	/// </summary>
	void Reset() override {
		currentIndex_ = 0;
		for (auto& child : children_) {
			child->Reset();
		}
	}

protected:

	// 子ノードのリスト
	std::vector<std::unique_ptr<BehaviorNode>> children_;

	// 現在実行中の子ノードのインデックス
	size_t currentIndex_ = 0;
};