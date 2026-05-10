#pragma once
#include "application/Entity/Behavior/BehaviorNode.h"
#include <string>

/**
 * @brief Blackboard に文字列をセットするアクションノード。
 *
 * Execute() は即座に Success を返す。
 * AIパラメータ（ウェイトJSONファイル名など）を
 * BT 経由で切り替えるために使う。
 */
class SetBlackboardStringNode : public BehaviorNode {
public:
	SetBlackboardStringNode(const std::string& key = "", const std::string& value = "", const std::string& name = "SetBBString");
	~SetBlackboardStringNode() override = default;

	/**
	 * @brief 指定キーに文字列をセットして Success を返す。
	 * @param blackboard 書き込み先
	 * @return BehaviorStatus::Success
	 */
	BehaviorStatus Execute(Blackboard& blackboard) override;

	/**
	 * @brief ステートレスなのでnop。
	 */
	void Reset() override;

	//=============================================================================
	// accessors
	//=============================================================================
	const std::string& GetKey() const { return key_; }
	void SetKey(const std::string& key) { key_ = key; }

	const std::string& GetStringValue() const { return value_; }
	void SetStringValue(const std::string& value) { value_ = value; }

private:
	// 書き込み先のBlackboardキー
	std::string key_;
	// セットする文字列値
	std::string value_;
};
