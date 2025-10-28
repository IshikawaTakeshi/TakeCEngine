#include "StringUtility.h"
#include <Windows.h>
#include <debugapi.h>
#include <iostream>


//=========================================================
// 文字列変換（std::string -> std::wstring）
//=========================================================
std::wstring StringUtility::ConvertString(const std::string& str) {
	if (str.empty()) {
		// 空文字列の場合は空のwstringを返す
		return std::wstring();
	}

	// 必要なバッファサイズを取得
	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		// 変換に失敗した場合
		throw std::runtime_error("Failed to convert string to wstring: MultiByteToWideChar failed.");
	}
	// 変換後の文字列を格納するためのバッファを確保
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

//=========================================================
// 文字列変換（std::wstring -> std::string）
//=========================================================
std::string StringUtility::ConvertString(const std::wstring& str) {
	if (str.empty()) {
		// 空文字列の場合は空のstringを返す
		return std::string();
	}

	// 必要なバッファサイズを取得
	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		// 変換に失敗した場合
		throw std::runtime_error("Failed to convert string to wstring: MultiByteToWideChar failed.");
	}

	// 変換後の文字列を格納するためのバッファを確保
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}