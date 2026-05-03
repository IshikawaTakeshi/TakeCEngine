#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>
#include <algorithm>
#include <imgui.h>
#include <cassert>

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

	/// <summary>
	/// ImGuiで全てのキーと値を表示・編集する
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 指定したキーが存在するか
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	bool HasKey(const std::string& key) const;

	/// <summary>
	/// 指定したキーを削除する
	/// </summary>
	/// <param name="key"></param>
	void RemoveKey(const std::string& key);

	/// <summary>
	/// 全てのキーと値をクリアする
	/// </summary>
	void Clear();

	//=========================================================================
	// accessors
	//=========================================================================

	float GetAsFloat(const std::string& key, float defaultValue = 0.0f) const;

	// 特定のキーへのアクセス
	template<typename T>
	T GetValue(const std::string& key) const;
	template<typename T>
	void SetValue(const std::string& key, const T& value);

	// 全てのデータへのアクセス
	std::unordered_map<std::string, BlackboardValue>& GetAllData() { return data_; }
	const std::unordered_map<std::string, BlackboardValue>& GetAllData() const { return data_; }

private:

	BlackboardValue CreateDefaultByIndex(int index) const;
private:

	//キーと値のペアを格納するマップ
	std::unordered_map<std::string, BlackboardValue> data_;

};

//-----------------------------------------------------------------------------
// valueの取得
//-----------------------------------------------------------------------------
template<typename T>
inline T Blackboard::GetValue(const std::string& key) const {
	auto it = data_.find(key);
	if (it != data_.end()) {
		if (std::holds_alternative<T>(it->second)) {
			return std::get<T>(it->second);
		}
		else {
			assert(false && "Type mismatch for key");
			return T(); // 型が違う場合はデフォルト値を返す
		}
	}
	else {
		assert(false && "Key not found");
		return T(); // キーが見つからない場合はデフォルト値を返す
	}
}

//-----------------------------------------------------------------------------
// valueの設定
//-----------------------------------------------------------------------------
template<typename T>
inline void Blackboard::SetValue(const std::string& key, const T& value) {
	data_[key] = value;
}
