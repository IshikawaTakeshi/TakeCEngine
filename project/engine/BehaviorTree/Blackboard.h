#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

namespace TakeC {

/// <summary>
/// Blackboardに格納できる、保存・比較可能な値の集合です。
/// </summary>
using BlackboardValue = std::variant<std::int32_t, float, bool, std::string>;

namespace Detail {

template<class T>
inline constexpr bool IsBlackboardValue =
	std::is_same_v<std::remove_cvref_t<T>, std::int32_t> ||
	std::is_same_v<std::remove_cvref_t<T>, float> ||
	std::is_same_v<std::remove_cvref_t<T>, bool> ||
	std::is_same_v<std::remove_cvref_t<T>, std::string>;

} // namespace Detail

/// <summary>
/// キー名と値型を組にして、Blackboardアクセス時の型を保証するキーです。
/// </summary>
template<class T>
struct BlackboardKey final {
	static_assert(Detail::IsBlackboardValue<T>, "T is not supported by BlackboardValue");

	std::string_view name;
};

/// <summary>
/// ビヘイビアツリーのノード間で共有する型付きデータを保持するクラスです。
/// Runtime機能だけを担当し、表示・編集処理には依存しません。
/// </summary>
class Blackboard final {
public:
	using Storage = std::unordered_map<std::string, BlackboardValue>;

	Blackboard() = default;
	~Blackboard() = default;

	Blackboard(const Blackboard&) = default;
	Blackboard& operator=(const Blackboard&) = default;
	Blackboard(Blackboard&&) noexcept = default;
	Blackboard& operator=(Blackboard&&) noexcept = default;

	/// <summary>
	/// キーが存在するかを調べます。
	/// </summary>
	[[nodiscard]] bool Contains(std::string_view key) const;

	/// <summary>
	/// キーを削除します。削除した場合はtrueを返します。
	/// </summary>
	bool Remove(std::string_view key);

	/// <summary>
	/// キー名を変更します。元キーがない場合や変更先が使用済みの場合はfalseを返します。
	/// </summary>
	bool Rename(std::string_view oldKey, std::string_view newKey);

	/// <summary>
	/// 全ての値を削除します。
	/// </summary>
	void Clear();

	/// <summary>
	/// 値を設定します。既存キーと型が異なる場合やキーが空の場合はfalseを返します。
	/// </summary>
	template<class T>
	bool Set(std::string_view key, T&& value);

	/// <summary>
	/// 型付きキーを使って値を設定します。
	/// </summary>
	template<class T>
	bool Set(const BlackboardKey<T>& key, T value);

	/// <summary>
	/// Editorやデシリアライズ処理向けに、既存の型を含めて値を置き換えます。
	/// </summary>
	bool SetOrReplace(std::string_view key, BlackboardValue value);

	/// <summary>
	/// 指定型の値を取得します。キーがない場合や型が異なる場合はnullptrを返します。
	/// </summary>
	template<class T>
	[[nodiscard]] T* TryGet(std::string_view key);

	template<class T>
	[[nodiscard]] const T* TryGet(std::string_view key) const;

	/// <summary>
	/// 型付きキーを使って値を取得します。
	/// </summary>
	template<class T>
	[[nodiscard]] T* TryGet(const BlackboardKey<T>& key);

	template<class T>
	[[nodiscard]] const T* TryGet(const BlackboardKey<T>& key) const;

	/// <summary>
	/// 値を取得し、取得できない場合は指定された既定値を返します。
	/// </summary>
	template<class T>
	[[nodiscard]] T GetOr(std::string_view key, T defaultValue) const;

	template<class T>
	[[nodiscard]] T GetOr(const BlackboardKey<T>& key, T defaultValue) const;

	/// <summary>
	/// 数値型の値をfloatへ変換して取得します。非数値型または未登録ならnulloptを返します。
	/// </summary>
	[[nodiscard]] std::optional<float> TryGetNumberAsFloat(std::string_view key) const;

	/// <summary>
	/// 読み取り専用の全エントリーを取得します。
	/// </summary>
	[[nodiscard]] const Storage& Entries() const noexcept { return data_; }

	/// <summary>
	/// 最後に値が変化した世代を取得します。
	/// </summary>
	[[nodiscard]] std::uint64_t Revision() const noexcept { return revision_; }

private:
	Storage data_;
	std::uint64_t revision_ = 0;
};

template<class T>
bool Blackboard::Set(std::string_view key, T&& value) {
	using ValueType = std::remove_cvref_t<T>;
	static_assert(Detail::IsBlackboardValue<ValueType>, "T is not supported by BlackboardValue");

	if (key.empty()) {
		return false;
	}

	auto it = data_.find(std::string(key));
	if (it == data_.end()) {
		data_.emplace(std::string(key), BlackboardValue{std::forward<T>(value)});
		++revision_;
		return true;
	}

	auto* current = std::get_if<ValueType>(&it->second);
	if (current == nullptr) {
		return false;
	}

	if (*current != value) {
		*current = std::forward<T>(value);
		++revision_;
	}
	return true;
}

template<class T>
bool Blackboard::Set(const BlackboardKey<T>& key, T value) {
	return Set<T>(key.name, std::move(value));
}

template<class T>
T* Blackboard::TryGet(std::string_view key) {
	static_assert(Detail::IsBlackboardValue<T>, "T is not supported by BlackboardValue");

	auto it = data_.find(std::string(key));
	return it == data_.end() ? nullptr : std::get_if<T>(&it->second);
}

template<class T>
const T* Blackboard::TryGet(std::string_view key) const {
	static_assert(Detail::IsBlackboardValue<T>, "T is not supported by BlackboardValue");

	auto it = data_.find(std::string(key));
	return it == data_.end() ? nullptr : std::get_if<T>(&it->second);
}

template<class T>
T* Blackboard::TryGet(const BlackboardKey<T>& key) {
	return TryGet<T>(key.name);
}

template<class T>
const T* Blackboard::TryGet(const BlackboardKey<T>& key) const {
	return TryGet<T>(key.name);
}

template<class T>
T Blackboard::GetOr(std::string_view key, T defaultValue) const {
	if (const T* value = TryGet<T>(key)) {
		return *value;
	}
	return defaultValue;
}

template<class T>
T Blackboard::GetOr(const BlackboardKey<T>& key, T defaultValue) const {
	return GetOr<T>(key.name, std::move(defaultValue));
}

} // namespace TakeC

// 既存のBehaviorTreeコードを段階的にTakeC名前空間へ移行するための互換用宣言。
using TakeC::Blackboard;
using TakeC::BlackboardKey;
using TakeC::BlackboardValue;
