#pragma once
#include <string>
#include <magic_enum.hpp>

// enumのみに制約をかける
template<typename T>
concept InputEnum = std::is_enum_v<T>;

namespace StringUtility {

	//コンバートストリング(wstring型)
	std::wstring ConvertString(const std::string& str);
	//コンバートストリング(string型)
	std::string ConvertString(const std::wstring& str);

	template<InputEnum Enum>
	std::string EnumToString(Enum e);

}


template<InputEnum Enum>
std::string StringUtility::EnumToString(Enum e) {
	return std::string(magic_enum::enum_name(e));
}	