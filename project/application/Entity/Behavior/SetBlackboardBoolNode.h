#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"
#include <string>

class SetBlackboardBoolNode : public BehaviorNode {
public:
	/// <summary>
	/// コンストラクタ / デストラクタ
	/// </summary>
	SetBlackboardBoolNode(const std::string& key = "", bool value = false, const std::string& name = "SetBBBool");
	~SetBlackboardBoolNode() override = default;

	/// <summary>
	/// ノードの実行
	/// </summary>
	BehaviorStatus Execute(Blackboard& blackboard) override;

	/// <summary>
	/// ノードのリセット
	/// </summary>
	void Reset() override;

	//=============================================================================
	// accessors
	//=============================================================================
	const std::string& GetKey() const { return key_; }
	void SetKey(const std::string& key) { key_ = key; }

	bool GetValue() const { return value_; }
	void SetValue(bool value) { value_ = value; }

private:
	std::string key_;
	bool value_;
};
