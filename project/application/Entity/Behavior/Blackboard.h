#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>
#include <algorithm>
#include <imgui.h>

//============================================================================
// Blackboard に格納できる値の型
//============================================================================
using BlackboardValue = std::variant<int, float, bool, std::string>;
//============================================================================
// Blackboard
//============================================================================
class Blackboard {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	Blackboard() = default;
	~Blackboard() = default;

	//=========================================================================
	// functions
	//=========================================================================

	bool HasKey(const std::string& key) const;
	void RemoveKey(const std::string& key);
	void Clear();

	//=========================================================================
	// accessors
	//=========================================================================
	template<typename T>
	T GetValue(const std::string& key) const {
		auto it = data_.find(key);
		if (it != data_.end()) {
			if (std::holds_alternative<T>(it->second)) {
				return std::get<T>(it->second);
			}
			else {
				throw std::runtime_error("Type mismatch for key: " + key);
			}
		}
		else {
			throw std::runtime_error("Key not found: " + key);
		}
	}

	template<typename T>
	void SetValue(const std::string& key, const T& value) {
		data_[key] = value;
	}

private:

	std::unordered_map<std::string, BlackboardValue> data_;

};

